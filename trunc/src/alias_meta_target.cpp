#include "stdafx.h"
#include "alias_meta_target.h"
#include "main_target.h"
#include "project.h"
#include "engine.h"
#include "feature_registry.h"

using namespace std;

namespace hammer{

alias_meta_target::alias_meta_target(hammer::project* p, const pstring& name, 
                                     const sources_decl& sources,
                                     const requirements_decl& req,
                                     const requirements_decl& usage_req) 
                                     : basic_meta_target(p, name, req, usage_req)
{
   this->sources(sources);
}
   
void alias_meta_target::instantiate(const main_target* owner, 
                                    const feature_set& build_request,
                                    std::vector<basic_target*>* result, 
                                    feature_set* usage_requirements) const
{
   if (owner != NULL)
      this->usage_requirements().eval(owner->properties(), usage_requirements);
   else // top level alias instantiation
   {
      vector<basic_target*> sources;
      sources_decl simple_targets;
      meta_targets_t meta_targets;
      split_sources(&simple_targets, &meta_targets, build_request);
      instantiate_meta_targets(meta_targets, build_request, NULL, &sources, usage_requirements);
      result->insert(result->end(), sources.begin(), sources.end());
   }
}

void alias_meta_target::transfer_sources(sources_decl* simple_targets, 
                                         meta_targets_t* meta_targets, 
                                         const feature_set& build_request) const
{
   split_sources(simple_targets, meta_targets, build_request);
}

}