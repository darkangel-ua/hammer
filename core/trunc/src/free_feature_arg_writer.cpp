#include "stdafx.h"
#include <hammer/core/free_feature_arg_writer.h>
#include <hammer/core/build_node.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature_def.h>
#include <hammer/core/feature.h>
#include <hammer/core/basic_meta_target.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/main_target.h>

namespace hammer{

free_feature_arg_writer::free_feature_arg_writer(const std::string& name, 
                                                 const feature_def& def,
                                                 const std::string& prefix, 
                                                 const std::string& suffix,
                                                 const std::string& delimiter,
                                                 const std::string& global_prefix,
                                                 const std::string& global_suffix)
   : argument_writer(name), feature_def_(def),
     prefix_(prefix), suffix_(suffix), delimiter_(delimiter),
     global_prefix_(global_prefix), global_suffix_(global_suffix)
{
}

void free_feature_arg_writer::write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const
{
   bool global_prefix_written = false;
   const feature_set& build_request = node.build_request();
   for(feature_set::const_iterator i = build_request.find(feature_def_.name()), last = build_request.end(); 
       i != last;)
   {
      if (!global_prefix_written)
      {
         output << global_prefix_;
         global_prefix_written = true;
      }

      if (feature_def_.attributes().path)
      {
         if(node.products_.empty()) 
            throw std::runtime_error("[free_feature_arg_writer] Can't write path feature for node without probucts.");

         location_t include_path((**i).get_path_data().target_->location() / (**i).value().to_string());
          include_path.normalize();
          include_path = relative_path(include_path, node.products_.front()->mtarget()->location());
          include_path.normalize();
          output << prefix_ << include_path.native_file_string() << suffix_ << delimiter_;
      }
      else
         output << prefix_ << (**i).value() << suffix_ << delimiter_;
      i = build_request.find(++i, feature_def_.name());
   }

   if (global_prefix_written)
      output << global_suffix_;
}

free_feature_arg_writer* free_feature_arg_writer::clone() const
{
   return new free_feature_arg_writer(*this);
}

}

