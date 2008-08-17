#pragma once

#include "meta_target.h"

namespace hammer
{
   class alias_meta_target : public basic_meta_target
   {
      public:
         alias_meta_target(hammer::project* p, const pstring& name, 
                           const sources_decl& sources,
                           const requirements_decl& req,
                           const requirements_decl& usage_req);
         virtual void instantiate(const main_target* owner, 
                                  const feature_set& build_request,
                                  std::vector<basic_target*>* result, 
                                  feature_set* usage_requirements) const;
      private:
         virtual void transfer_sources(sources_decl* simple_targets, 
                                       meta_targets_t* meta_targets,
                                       const feature_set& build_request) const;
   };
}