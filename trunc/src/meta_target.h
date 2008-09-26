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
         virtual main_target* construct_main_target(const feature_set* properties) const = 0;
      
      private:
         void instantiate_meta_targets(sources_decl& simple_targets,
                                       feature_set& usage_requirments,
                                       std::vector<basic_target*>& instantiated_targets,
                                       const meta_targets_t& meta_targets,
                                       const feature_set& build_request,
                                       const main_target& owner_for_new_targets) const;
   };
}
