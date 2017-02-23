#include "stdafx.h"
#include <hammer/core/searched_lib_meta_target.h>
#include <hammer/core/searched_lib_main_target.h>
#include <hammer/core/engine.h>

namespace hammer{

searched_lib_meta_target::searched_lib_meta_target(hammer::project* p, 
                                                   const std::string& name,
                                                   const std::string& lib_name,
                                                   const requirements_decl& props,
                                                   const requirements_decl& usage_req,
                                                   const target_type& t)
   :
    typed_meta_target(p, name, props, usage_req, t),
    lib_name_(lib_name)
 {}

main_target* 
searched_lib_meta_target::construct_main_target(const main_target* owner, 
                                                const feature_set* properties) const
{
   main_target* mt = new searched_lib_main_target(this, 
                                                  name(),
                                                  lib_name_,
                                                  properties,
                                                  type());
   return mt;
}

}

