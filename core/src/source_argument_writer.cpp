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
                                               bool exact_type,
                                               output_strategy os,
                                               const std::string& quoting_string,
                                               const std::string& prefix)
   : targets_argument_writer(name, t), 
     exact_type_(exact_type),
     output_strategy_(os),
     quoting_string_(quoting_string),
     prefix_(prefix)
{
}

argument_writer* source_argument_writer::clone() const
{
   return new source_argument_writer(*this);
}

bool source_argument_writer::accept(const basic_target& source) const
{
   if (exact_type_)
      return source.type() == this->source_type();
   else
      return source.type().equal_or_derived_from(this->source_type());
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

         switch(output_strategy_)
         {
            case RELATIVE_TO_MAIN_TARGET:
            {
               location_t source_path = relative_path(i->source_target_->location(), i->source_target_->get_main_target()->location());
               source_path /= i->source_target_->name();
               source_path.normalize();
               output << quoting_string_ << prefix_ << source_path.string() << quoting_string_;
               break;
            }

            case FULL_PATH:
            {
               location_t source_path = i->source_target_->location() / i->source_target_->name();
               source_path.normalize();
               output << quoting_string_ << prefix_ << source_path.string() << quoting_string_;
               break;
            }

            case RELATIVE_TO_WORKING_DIR:
            {
               location_t source_path = relative_path(i->source_target_->location(), environment.working_directory(*i->source_target_));
               source_path /= i->source_target_->name();
               source_path.normalize();
               output << quoting_string_ << prefix_ << source_path.string() << quoting_string_;
               break;
            }

            case WITHOUT_PATH:
            {
               location_t source_path(i->source_target_->name());
               output << quoting_string_ << prefix_ << source_path.string() << quoting_string_;
               break;
            }

            default:
               throw std::runtime_error("Unknown output strategy");
         }
      }
   }
}

}
