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
                                 const pstring& name,
                                 const requirements_decl& req,
                                 const requirements_decl& usage_req) 
                                : 
                                 meta_target(p, name, req, usage_req)
{
}

main_target* lib_meta_target::construct_main_target(const main_target* owner, const feature_set* properties) const
{
   main_target* result = 0;
   // check for searched lib
   if (properties->find("name") != properties->end() ||
       properties->find("file") != properties->end())
   {
      if (!sources().empty())
         throw std::runtime_error("lib target can't have sources when <file> or <name> specified");

      result = new searched_lib_main_target(this, 
                                            name(), 
                                            properties,
                                            project()->engine()->targets_pool());
   }
   else
   {
      const feature& link = properties->get("link");
      const type* target_type = 0;
      if (link.value() == "static")
         target_type = &this->project()->engine()->get_type_registry().get(types::STATIC_LIB);
      else
         target_type = &this->project()->engine()->get_type_registry().get(types::SHARED_LIB);

      result = new main_target(this, 
                               name(), 
                               target_type, 
                               properties,
                               project()->engine()->targets_pool());
   }

   return result;
}

}