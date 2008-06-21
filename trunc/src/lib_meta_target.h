#pragma once
#include "meta_target.h"

namespace hammer
{
   class lib_meta_target : public meta_target
   {
      public:
         lib_meta_target(hammer::project* p, const pstring& name, 
                         const requirements_decl& req,
                         const requirements_decl& usage_req);
      
      protected:
/*
         virtual void instantiate(const main_target* owner, 
                                  const feature_set& build_request,
                                  std::vector<basic_target*>* result, 
                                  feature_set* usage_requirements) const;
*/
         virtual main_target* construct_main_target(const feature_set* properties) const;
   };
}