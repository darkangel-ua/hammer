#include "stdafx.h"
#include <hammer/core/actuality_checker.h>
#include <hammer/core/timestamp_info.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/scaner.h>
#include <hammer/core/scaner_manager.h>
#include <hammer/core/engine.h>

using namespace boost::date_time;
using namespace boost::posix_time;

namespace hammer{

std::size_t actuality_checker::check(nodes_t& nodes)
{
   std::size_t result = 0;
   ptime max_node_time;
   for(nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
      check(max_node_time, result, **i);

   return result;
}

scanner_context& actuality_checker::get_scanner_context(const type& t, const scanner& s)
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

bool actuality_checker::check(boost::posix_time::ptime& max_target_time, std::size_t& nodes_to_update, build_node& node)
{
   if (node.up_to_date() != boost::tribool::indeterminate_value)
   {
      max_target_time = (std::max)(node.timestamp(), max_target_time);
      if (node.up_to_date() == boost::tribool::true_value)
         return false;
      else
         return true;
   }

   bool some_need_to_be_updated = false;
   ptime sources_max_time(neg_infin);
   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
      some_need_to_be_updated = check(sources_max_time, nodes_to_update, *i->source_node_) || some_need_to_be_updated;

   // check for dependencies
   for(build_node::nodes_t::const_iterator i = node.dependencies_.begin(), last = node.dependencies_.end(); i != last; ++i)
      some_need_to_be_updated = check(sources_max_time, nodes_to_update, **i) || some_need_to_be_updated;

   ptime products_max_time(boost::date_time::neg_infin);
   for(build_node::targets_t::const_iterator i = node.products_.begin(), last = node.products_.end(); i != last; ++i)
   {
      const timestamp_info_t& time_info = (**i).timestamp_info();
      if (time_info.is_unknown_)
         (**i).timestamp_info(timestamp_info_t::recheck);

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
         ptime scanner_timestamp = scanner->process(**i, get_scanner_context((**i).type(), *scanner));
         
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
      nodes_to_update += node.products_.size();
      return true;
   }
   else
   {
      ptime this_max_target_time = (std::max)(sources_max_time, products_max_time);
      node.timestamp(this_max_target_time);
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
