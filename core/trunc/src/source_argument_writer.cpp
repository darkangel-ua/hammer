#include "stdafx.h"
#include <hammer/core/source_argument_writer.h>
#include <hammer/core/build_node.h>
#include <hammer/core/main_target.h>
#include <hammer/core/target_type.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/build_environment.h>

namespace hammer{

source_argument_writer::source_argument_writer(const std::string& name, 
                                               const target_type& t,
                                               bool write_full_path,
                                               const std::string& quoting_string)
   : targets_argument_writer(name, t), 
     write_full_path_(write_full_path),
     quoting_string_(quoting_string)
{
}

argument_writer* source_argument_writer::clone() const
{
   return new source_argument_writer(*this);
}

bool source_argument_writer::accept(const basic_target& source) const
{
   return  source.type().equal_or_derived_from(this->source_type());
}

void source_argument_writer::write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const
{
   bool first = true;
   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
   {
      if (accept(*i->source_target_))
      {
         if (!first)
            output << ' ';
         else
            first = false;

         if (write_full_path_)
         {
            location_t source_path = i->source_target_->location() / i->source_target_->name().to_string();
            source_path.normalize();
            output << quoting_string_ << source_path.native_file_string() << quoting_string_; 
         }
         else
         {
            location_t source_path = relative_path(i->source_target_->location(), i->source_target_->get_main_target()->location());
            source_path.normalize();
            source_path /= i->source_target_->name().to_string();
            output << quoting_string_ << source_path.native_file_string() << quoting_string_;
         }
      }
   }
}

}
