#include "stdafx.h"
#include <hammer/core/actuality_checker.h>
#include <hammer/core/timestamp_info.h>
#include <hammer/core/basic_target.h>

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
