#include "stdafx.h"
#include "lib_meta_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "types.h"
#include "feature.h"
#include "feature_set.h"
#include "main_target.h"
#include "searched_lib_main_target.h"

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
   main_target* result = 0;
   // check for searched lib
   if (properties->find("name") != properties->end())
   {
      result = new(project()->engine()->targets_pool()) 
                   searched_lib_main_target(this, 
                                            name(), 
                                            properties,
                                            project()->engine()->targets_pool());
   }
   else
   {
      const feature* link = properties->get("link");
      const type* target_type = 0;
      if (link->value() == "static")
         target_type = &this->project()->engine()->get_type_registry().resolve_from_name(types::STATIC_LIB);
      else
         target_type = &this->project()->engine()->get_type_registry().resolve_from_name(types::SHARED_LIB);

      result = new(project()->engine()->targets_pool()) 
                   main_target(this, 
                              name(), 
                              target_type, 
                              properties,
                              project()->engine()->targets_pool());
   }

   return result;
}

/*
void lib_meta_target::instantiate(const main_target* owner, 
                                  const feature_set& build_request,
                                  std::vector<basic_target*>* result, 
                                  feature_set* usage_requirements) const
{
   meta_target::instantiate(owner, build_request, result, usage_requirements);
}
*/

}