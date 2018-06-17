#include "stdafx.h"
#include <hammer/core/lib_meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/feature.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/main_target.h>
#include <hammer/core/searched_lib_main_target.h>

namespace hammer{

lib_meta_target::lib_meta_target(hammer::project* p, 
                                 const std::string& name,
                                 const requirements_decl& req,
                                 const requirements_decl& usage_req) 
                                : 
                                 meta_target(p, name, req, usage_req)
{
}

main_target* lib_meta_target::construct_main_target(const main_target* owner, const feature_set* properties) const
{
   main_target* result = 0;
   const feature& link = properties->get("link");
   const target_type* target_type = 0;
   if (link.value() == "static")
      target_type = &get_engine().get_type_registry().get(types::STATIC_LIB);
   else
      target_type = &get_engine().get_type_registry().get(types::SHARED_LIB);

   result = new main_target(this, 
                            name(), 
                            target_type, 
                            properties);

   return result;
}

}
