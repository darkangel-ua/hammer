#include "stdafx.h"
#include <hammer/core/alias_meta_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/build_request.h>

using namespace std;

namespace hammer{

alias_meta_target::alias_meta_target(hammer::project* p, const std::string& name,
                                     const sources_decl& sources,
                                     const requirements_decl& req,
                                     const requirements_decl& usage_req) 
                                     : basic_meta_target(p, name, req, usage_req)
{
   this->sources(sources);
}
   
void alias_meta_target::instantiate_impl(instantiation_context& ctx,
                                         const main_target* owner,
                                         const feature_set& build_request,
                                         std::vector<basic_target*>* result, 
                                         feature_set* usage_requirements) const
{
   if (owner) {
      // compute usage requirements based only on build request
      // our own requirements is just for target selection - that's how alias works
      this->usage_requirements().eval(build_request, usage_requirements);

      feature_set& sources_as_features = *get_engine().feature_registry().make_set();

      auto sources = this->sources();
      apply_project_dependencies(sources, *this);

      for (const source_decl& sd : sources) {
         source_decl new_sd = sd;

         // apply build request to a target
         new_sd.build_request_join(build_request);

         if (looks_like_local_target_ref(sd))
            new_sd.set_locals_allowed(true);

         feature_ref f = get_engine().feature_registry().create_feature("source", new_sd);
         sources_as_features.join(f);
      }

      adjust_dependency_features_sources(sources_as_features);
      usage_requirements->join(sources_as_features);
   } else {
      // top level alias instantiation
      auto sources = this->sources();
      apply_project_dependencies(sources, *this);

      sources_decl simple_targets;
      meta_targets_t meta_targets;
      split_sources(&simple_targets, &meta_targets, sources, build_request);
      instantiate_meta_targets(ctx, meta_targets, nullptr, result, usage_requirements);
   }
}

}
