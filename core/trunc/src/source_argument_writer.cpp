#include "stdafx.h"
#include <hammer/core/source_argument_writer.h>
#include <hammer/core/build_node.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/type.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/build_environment.h>

namespace hammer{

source_argument_writer::source_argument_writer(const std::string& name, 
                                               const type& t)
   : targets_argument_writer(name, t)
{
}

argument_writer* source_argument_writer::clone() const
{
   return new source_argument_writer(*this);
}

void source_argument_writer::write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const
{
   bool first = true;
   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
   {
      if (i->source_target_->type().equal_or_derived_from(this->source_type()))
      {
         if (!first)
            output << ' ';
         else
            first = false;

         location_t source_path = relative_path(i->source_target_->location(), environment.current_directory()) / i->source_target_->name().to_string();
         source_path.normalize();
         output << source_path.native_file_string();
      }
   }
}

}
