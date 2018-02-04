#include "stdafx.h"
#include <hammer/core/actuality_checker.h>
#include <hammer/core/timestamp_info.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/scaner.h>
#include <hammer/core/scaner_manager.h>
#include <hammer/core/engine.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::date_time;
using namespace boost::posix_time;

namespace hammer{

std::size_t actuality_checker::check(nodes_t& nodes)
{
   std::size_t result = 0;
   ptime max_node_time(neg_infin);
   for(nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
      check(max_node_time, result, **i);

   return result;
}

scanner_context& actuality_checker::get_scanner_context(const target_type& t, const scanner& s)
{
   scanner_contexts_t::const_iterator i = scanner_contexts_.find(&t);
   if (i == scanner_contexts_.end())
   {
      scanner_contexts_.insert(std::make_pair(&t, boost::shared_ptr<scanner_context>(s.create_context(env_))));
      return get_scanner_context(t, s);
   }
   else
      return *i->second;
}

// go deep in down nodes and mark all of they as needed to update, including dependency nodes
void actuality_checker::mark_to_update(build_node& node, 
                                       std::size_t& nodes_to_update, 
                                       const main_target& products_owner)
{
   for(build_node::nodes_t::iterator i = node.down_.begin(), last = node.down_.end(); i != last; ++i)
      if (&(**i).products_owner() == &products_owner)
      {
         (**i).up_to_date(boost::tribool::false_value);
         (**i).timestamp(pos_infin);
         mark_to_update(**i, nodes_to_update, products_owner);
         ++nodes_to_update;
      }
      else
      {
         ptime max_node_time(neg_infin);
         check(max_node_time, nodes_to_update, **i);
      }
   
   // We should mark dependencies also if they was generated as part of main_target generation process
   // just as direct dependencies. PCH or QT_UIC go to deps not to sources.
   for(build_node::nodes_t::iterator i = node.dependencies_.begin(), last = node.dependencies_.end(); i != last; ++i)
      if (&(**i).products_owner() == &products_owner)
      {
         (**i).up_to_date(boost::tribool::false_value);
         (**i).timestamp(pos_infin);
         mark_to_update(**i, nodes_to_update, products_owner);
         ++nodes_to_update;
      }
      else
      {
         ptime max_node_time(neg_infin);
         check(max_node_time, nodes_to_update, **i);
      }
}

bool actuality_checker::check(boost::posix_time::ptime& max_target_time, std::size_t& nodes_to_update, build_node& node)
{
   assert(max_target_time != not_a_date_time);

   if (node.up_to_date() != boost::tribool::indeterminate_value)
   {
      max_target_time = (std::max)(node.timestamp(), max_target_time);
      assert(node.timestamp() != not_a_date_time);
      if (node.up_to_date() == boost::tribool::true_value)
         return false;
      else
         return true;
   }

   // check for dependencies
   bool dependency_need_to_be_updated = false;
   ptime dependency_max_time(neg_infin);
   for(build_node::nodes_t::const_iterator i = node.dependencies_.begin(), last = node.dependencies_.end(); i != last; ++i)
      dependency_need_to_be_updated = check(dependency_max_time, nodes_to_update, **i) || dependency_need_to_be_updated;

   if (dependency_need_to_be_updated)
      mark_to_update(node, nodes_to_update, node.products_owner());

   ptime sources_max_time(neg_infin);
   bool some_need_to_be_updated = dependency_need_to_be_updated;
   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
   {
      // we skip direct sources that is dependency too. Good example is pch header that can be found in dependency and direct sources
      // we need this because when pch header has been rebuilt sources must be rebuild too.
      if (node.dependencies_.empty() ||
          find(node.dependencies_.begin(), node.dependencies_.end(), i->source_node_) == node.dependencies_.end())
      {
         some_need_to_be_updated = check(sources_max_time, nodes_to_update, *i->source_node_) || some_need_to_be_updated;
      }
   }

/*
   // if we have some sources and they older then dependencies than
   // we should mark all nodes down from here to update, 
   // because we should build dependencies first and than sources
   if (!dependency_need_to_be_updated &&
       !node.sources_.empty() && 
       sources_max_time < dependency_max_time)
   {
      mark_to_update(node, nodes_to_update, node.products_owner());
   }
*/

   sources_max_time = (std::max)(sources_max_time, dependency_max_time);
   ptime products_max_time(boost::date_time::neg_infin);
   for(build_node::targets_t::const_iterator i = node.products_.begin(), last = node.products_.end(); i != last; ++i)
   {
	   const timestamp_info_t& time_info = (**i).timestamp_info();

      // product is missing?
      if (time_info.timestamp_ == neg_infin)
      {
         products_max_time = time_info.timestamp_;
         break;
      }

      products_max_time = (std::max)(products_max_time, time_info.timestamp_);

      const scanner* scanner = engine_.scanner_manager().find((**i).type());
      if (scanner != NULL)
      {
         ptime scanner_timestamp = scanner->process(**i, node, get_scanner_context((**i).type(), *scanner));

         // scanner say can't find some dependencies?
         if (scanner_timestamp == neg_infin)
         {
            products_max_time = neg_infin;
            break;
         }

         products_max_time = (std::max)(products_max_time, scanner_timestamp);
      }
   }

   if (products_max_time == neg_infin)
   {
      node.up_to_date(boost::tribool::false_value);
      node.timestamp(products_max_time);
      assert(node.timestamp() != not_a_date_time);
      nodes_to_update += node.products_.size();
      return true;
   }
   else
   {
      ptime this_max_target_time = (std::max)(sources_max_time, products_max_time);
      node.timestamp(this_max_target_time);
      assert(node.timestamp() != not_a_date_time);
      max_target_time = (std::max)(max_target_time, this_max_target_time);

      if (sources_max_time > products_max_time ||
          some_need_to_be_updated) // if some sources or dependencies not up to date than result != 0
      {
         node.up_to_date(boost::tribool::false_value);
         nodes_to_update += node.products_.size();
         return true;
      }
      else
      {
         node.up_to_date(boost::tribool::true_value);
         return false;
      }
   }
}

}
