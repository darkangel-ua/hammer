#include "stdafx.h"
#include "lib_meta_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "types.h"
#include "feature.h"
#include "feature_set.h"
#include "main_target.h"

namespace hammer{

lib_meta_target::lib_meta_target(hammer::project* p, 
                                 const pstring& name,
                                 const requirements_decl& req,
                                 const requirements_decl& usage_req) 
                                : 
                                 meta_target(p, name, req, usage_req)
{
}

main_target* lib_meta_target::construct_main_target(const feature_set* properties) const
{
   feature_set::const_iterator link = properties->find("link");
   const type* target_type = 0;
   if (link != properties->end())
   {
      if ((*link)->value() == "static")
         target_type = &this->project()->engine()->get_type_registry().resolve_from_name(types::STATIC_LIB);
      else
         target_type = &this->project()->engine()->get_type_registry().resolve_from_name(types::SHARED_LIB);
   }

   main_target* mt = new(project()->engine()->targets_pool()) 
                         main_target(this, 
                                    name(), 
                                    target_type, 
                                    properties,
                                    project()->engine()->targets_pool());
   return mt;
}

}