#include "stdafx.h"
#include <hammer/core/header_lib_meta_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/types.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/header_lib_main_target.h>

using namespace std;

namespace hammer{

header_lib_meta_target::header_lib_meta_target(hammer::project* p,
                                               const pstring& name,
                                               const requirements_decl& requirements,
                                               const requirements_decl& usage_requirements)
   : basic_meta_target(p, name, requirements, usage_requirements)
{

}

void header_lib_meta_target::instantiate_impl(const main_target* owner,
                                              const feature_set& build_request,
                                              std::vector<basic_target*>* result,
                                              feature_set* usage_requirements) const
{
   feature_set* build_request_for_sourses = get_engine()->feature_registry().make_set();
   build_request_for_sourses->copy_propagated(build_request);

   // now we split meta-targets from simple targets
   sources_decl simple_targets;
   sources_decl non_simple_targets;
   for(const source_decl& sd : sources()) {
      if (sd.type())
         simple_targets.push_back(sd);
      else
         non_simple_targets.push_back(sd);
   }

   // and push non-simple as <source> to usage requirements same as alias target do
   feature_set& sources_as_features = *get_engine()->feature_registry().make_set();
   for (const source_decl& sd : non_simple_targets) {
      feature* f = get_engine()->feature_registry().create_feature("source", "");
      source_decl new_sd = sd;

      // apply build request to a target
      if (new_sd.properties())
         new_sd.properties()->join(*build_request_for_sourses);
      else
         new_sd.properties(build_request_for_sourses->clone());

      f->set_dependency_data(new_sd, this);

      sources_as_features.join(f);
   }

   adjust_dependency_features_sources(sources_as_features, *this);
   usage_requirements->join(sources_as_features);

   feature_set* mt_fs = build_request.clone();
   requirements().eval(build_request, mt_fs, usage_requirements);

   main_target* mt = new header_lib_main_target(this,
                                                name(),
                                                &get_engine()->get_type_registry().get(types::HEADER_LIB),
                                                mt_fs,
                                                get_engine()->targets_pool());
   mt_fs = mt->properties().clone();

   vector<basic_target*> instantiated_simple_targets;
   instantiate_simple_targets(simple_targets, *mt_fs, *mt, &instantiated_simple_targets);
   mt->sources(instantiated_simple_targets);
   mt->properties(mt_fs);

   result->push_back(mt);

   // we just transfer downstream usage requirements to upstream + own
   this->usage_requirements().eval(owner->properties(), usage_requirements);
}

}
