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
      result += check(max_node_time, **i);

   return result;
}

scanner_context& actuality_checker::get_scanner_context(const type& t, const scanner& s)
{
   scanner_contexts_t::const_iterator i = scanner_contexts_.find(&t);
   if (i == scanner_contexts_.end())
   {
      scanner_contexts_.insert(std::make_pair(&t, boost::shared_ptr<scanner_context>(s.create_context())));
      return get_scanner_context(t, s);
   }
   else
      return *i->second;
}

std::size_t actuality_checker::check(boost::posix_time::ptime& max_target_time, build_node& node)
{
   std::size_t result = 0;
   ptime sources_max_time(neg_infin);
   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
      result += check(sources_max_time, *i->source_node_);

   // check for dependencies
   for(build_node::nodes_t::const_iterator i = node.dependencies_.begin(), last = node.dependencies_.end(); i != last; ++i)
      result += check(sources_max_time, **i);

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
         ptime scanner_timestamp = scanner->process((**i), get_scanner_context((**i).type(), *scanner));
         
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
      node.up_to_date(false);
      result += node.products_.size();
   }
   else
   {
      max_target_time = (std::max)(sources_max_time, products_max_time);
      if (sources_max_time > products_max_time ||
          result != 0) // if some sources or dependencies not up to date than result != 0
      {
         node.up_to_date(false);
         result += node.products_.size();
      }
      else
         node.up_to_date(true);
   }

   return result;
}

}
