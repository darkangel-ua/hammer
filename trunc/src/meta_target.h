#pragma once

#include "basic_meta_target.h"

namespace hammer
{
   class basic_target;
   class feature_set;
   class type;
   class generated_target;

   class meta_target : public basic_meta_target
   {
      public:
         meta_target(hammer::project* p, const pstring& name, 
                     const requirements_decl& props,
                     const requirements_decl& usage_req);
         virtual void instantiate(const main_target* owner, 
                                  const feature_set& build_request, 
                                  std::vector<basic_target*>* result, 
                                  feature_set* usage_requirements) const;

         virtual ~meta_target(){}

      protected:
         virtual const type* instantiate_type(const feature_set& fs) const = 0;
   };
}
