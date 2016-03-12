#include "stdafx.h"
#include <stdexcept>
#include <hammer/core/searched_lib_main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/file_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/searched_lib_target.h>
#include <hammer/core/target_type.h>
#include <hammer/core/file_target.h>

namespace hammer{

searched_lib_main_target::searched_lib_main_target(const hammer::meta_target* mt, 
                                                   const pstring& name, 
                                                   const pstring& lib_name, 
                                                   const feature_set* props,
                                                   const target_type& t,
                                                   pool& p)
   : 
    main_target(mt, name, &mt->get_engine()->get_type_registry().get(types::SEARCHED_LIB), props, p), 
    type_(&t),
    lib_name_(lib_name)
{
}

std::vector<boost::intrusive_ptr<hammer::build_node> > 
searched_lib_main_target::generate() const
{
   boost::intrusive_ptr<hammer::build_node> result(new hammer::build_node(*this, true));
   basic_target* t = type_->equal_or_derived_from(get_engine()->get_type_registry().get(types::SEARCHED_LIB)) 
                     ? 
                      new searched_lib_target(this, lib_name_, type_, &properties()) 
                     : 
                      new file_target(this, lib_name_, type_, &properties())
                     ;
   result->products_.push_back(t);
   result->targeting_type_ = this->type_;

   generate_and_add_dependencies(*result);

   return std::vector<boost::intrusive_ptr<hammer::build_node> >(1, result);
}

}
