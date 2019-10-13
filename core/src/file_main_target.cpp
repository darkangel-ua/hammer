#include "stdafx.h"
#include <hammer/core/file_main_target.h>
#include <hammer/core/source_build_target.h>

namespace hammer{

file_main_target::file_main_target(const basic_meta_target* mt,
                                   const std::string& name,
                                   const std::string& filename,
                                   const feature_set* props,
                                   const target_type& t)
   : 
    main_target(mt, name, &t, props),
    type_(&t),
    filename_(filename)
{

}

build_nodes_t
file_main_target::generate_impl() const
{
   build_node_ptr result{new hammer::build_node{*this, true, build_action_ptr{}}};
   location_t l_filename = filename_;
   if (!l_filename.has_root_path())
      l_filename = (location() / l_filename).normalize();

   basic_build_target* t = new source_build_target(this, l_filename.filename().string(), l_filename.branch_path(), type_, &properties());
   result->products_.push_back(t);
   result->targeting_type_ = this->type_;

   return std::vector<boost::intrusive_ptr<hammer::build_node> >(1, result);
}

}

