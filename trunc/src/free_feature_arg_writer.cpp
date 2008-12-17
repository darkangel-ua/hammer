#include "stdafx.h"
#include "free_feature_arg_writer.h"
#include "build_node.h"
#include "feature_set.h"
#include "feature_def.h"
#include "feature.h"
#include "basic_meta_target.h"
#include "build_environment.h"
#include "fs_helpers.h"

namespace hammer{

free_feature_arg_writer::free_feature_arg_writer(const std::string& name, 
                                                 const feature_def& def,
                                                 const std::string& prefix, 
                                                 const std::string& suffix)
   : argument_writer(name), feature_def_(def),
     prefix_(prefix), suffix_(suffix)
{
}

void free_feature_arg_writer::write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const
{
   const feature_set& build_request = node.build_request();
   for(feature_set::const_iterator i = build_request.find(feature_def_.name()), last = build_request.end(); 
       i != last;)
   {
      if (feature_def_.attributes().path)
      {
         location_t include_path((**i).get_path_data().target_->location() / (**i).value().to_string());
         include_path.normalize();
         include_path = relative_path(include_path, environment.current_directory());
         output << prefix_ << include_path.native_file_string() << suffix_ << ' ';
      }
      else
         output << prefix_ << (**i).value() << suffix_ << ' ';
      i = build_request.find(++i, feature_def_.name());
   }
}

free_feature_arg_writer* free_feature_arg_writer::clone() const
{
   return new free_feature_arg_writer(*this);
}

}

