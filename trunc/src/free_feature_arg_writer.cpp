#include "stdafx.h"
#include "free_feature_arg_writer.h"
#include "build_node.h"
#include "feature_set.h"
#include "feature_def.h"
#include "feature.h"

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
      output << prefix_ << (**i).value() << suffix_ << ' ';
      i = build_request.find(++i, feature_def_.name());
   }
}

free_feature_arg_writer* free_feature_arg_writer::clone() const
{
   return new free_feature_arg_writer(*this);
}

}

