#include "stdafx.h"
#include <stdexcept>
#include <hammer/core/searched_lib_main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/virtual_build_target.h>
#include <hammer/core/source_build_target.h>
#include <hammer/core/target_type.h>

namespace hammer{

searched_lib_main_target::searched_lib_main_target(const basic_meta_target* mt,
                                                   const std::string& name,
                                                   const std::string& lib_name,
                                                   const feature_set* props,
                                                   const target_type& t)
   : 
    main_target(mt, name, &mt->get_engine()->get_type_registry().get(types::SEARCHED_LIB), props),
    type_(&t),
    lib_name_(lib_name)
{
}

std::vector<boost::intrusive_ptr<hammer::build_node> > 
searched_lib_main_target::generate() const
{
   boost::intrusive_ptr<hammer::build_node> result(new hammer::build_node(*this, true));
   basic_build_target* t;
   if (type_->equal_or_derived_from(get_engine()->get_type_registry().get(types::SEARCHED_LIB)))
      t = new virtual_build_target(this, lib_name_, type_, &properties(), true);
   else {
      location_t l(lib_name_);
      if (!l.has_root_path())
         l = (location() / l).normalize();
      t = new source_build_target(this, l.filename().string(), l.branch_path().string(), type_, &properties());
   }
   result->products_.push_back(t);
   result->targeting_type_ = this->type_;

   generate_and_add_dependencies(*result);

   return std::vector<boost::intrusive_ptr<hammer::build_node> >(1, result);
}

}
