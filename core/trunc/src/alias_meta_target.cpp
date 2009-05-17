#include "stdafx.h"
#include <hammer/core/alias_meta_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>

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
   
void alias_meta_target::instantiate_impl(const main_target* owner, 
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
      split_sources(&simple_targets, &meta_targets, this->sources(), build_request);
      instantiate_meta_targets(meta_targets, build_request, NULL, &sources, usage_requirements);
      result->insert(result->end(), sources.begin(), sources.end());
   }
}

void alias_meta_target::transfer_sources(sources_decl* simple_targets, 
                                         meta_targets_t* meta_targets, 
                                         const feature_set& build_request,
                                         const feature_set* additional_build_properties) const
{
   if (additional_build_properties == NULL)
      split_sources(simple_targets, meta_targets, sources(), build_request);
   else
   {
      sources_decl new_simple_targets;
      meta_targets_t new_meta_targets;
      split_sources(&new_simple_targets, &new_meta_targets, sources(), build_request);
      
      simple_targets->add_to_source_properties(*additional_build_properties);
      
      for(meta_targets_t::iterator i = new_meta_targets.begin(), last = new_meta_targets.end(); i != last; ++i)
         if (i->second != NULL)
            i->second->join(*additional_build_properties);
         else
            i->second = additional_build_properties;

      meta_targets->insert(meta_targets->end(), new_meta_targets.begin(), new_meta_targets.end());
   }

   basic_meta_target::transfer_sources(simple_targets, meta_targets, 
                                       build_request, additional_build_properties);
}

}