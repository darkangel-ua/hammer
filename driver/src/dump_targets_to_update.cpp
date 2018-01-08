#include "stdafx.h"
#include "dump_targets_to_update.h"
#include <hammer/core/basic_build_target.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace hammer;

static std::ostream& put_indent(std::ostream& os, int indent)
{
   for(; indent > 0; --indent)
      os.put(' ');

   return os;
}

static void dump(std::ostream& os, 
                 const build_node& node, 
                 hammer::build_environment& environment, 
                 int indent)
{
   if (node.up_to_date() == boost::tribool::true_value)
      return;

   bool sources_up_to_date = true;
   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
   {
      if (i->source_node_->up_to_date() != boost::tribool::true_value)
      {
         sources_up_to_date = false;
         put_indent(os, indent) << "[s]" << i->source_target_->name() << " " << to_simple_string(i->source_target_->timestamp_info().timestamp_) << std::endl;
         dump(os, *i->source_node_, environment, indent + 1);
      }
   }

   bool dependencies_up_to_date = true;
   for(build_node::nodes_t::const_iterator i = node.dependencies_.begin(), last = node.dependencies_.end(); i != last; ++i)
   {
      if ((**i).up_to_date() != boost::tribool::true_value)
      {
         dependencies_up_to_date = false;
         put_indent(os, indent) << "[d]" << std::endl;
         dump(os, **i, environment, indent + 1);
      }
   }

   for(build_node::targets_t::const_iterator i = node.products_.begin(), last = node.products_.end(); i != last; ++i)
      put_indent(os, indent) << "[p]" << (**i).name() << " " << to_simple_string((**i).timestamp_info().timestamp_) << std::endl;
}

void dump_targets_to_update(std::ostream& os, 
                            const build_node::nodes_t& nodes,
                            hammer::build_environment& environment)
{
   for(build_node::nodes_t::const_iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
      dump(os, **i, environment, 0);
}

