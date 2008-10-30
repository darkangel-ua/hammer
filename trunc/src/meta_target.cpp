#include "stdafx.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "source_target.h"
#include "main_target.h"
#include "feature_set.h"
#include "feature_registry.h"
#include "feature.h"
#include "location.h"
#include "requirements_decl.h"

using namespace std;

namespace hammer{
   meta_target::meta_target(hammer::project* p, 
                            const pstring& name, 
                            const requirements_decl& props, 
                            const requirements_decl& usage_req) 
                           : basic_meta_target(p, name, props, usage_req)
   {
      requirements().insert_infront(p->requirements());
      usage_requirements().insert_infront(p->usage_requirements());
   }

   void meta_target::instantiate_meta_targets(sources_decl& simple_targets,
                                              std::vector<basic_target*>& instantiated_meta_targets,
                                              feature_set& usage_requirements,
                                              const meta_targets_t& meta_targets,
                                              const feature_set& build_request,
                                              const main_target& owner_for_new_targets) const
   {
      feature_set& local_usage_requirements = *project()->engine()->feature_registry().make_set();
      basic_meta_target::instantiate_meta_targets(meta_targets, build_request, &owner_for_new_targets, 
                                                  &instantiated_meta_targets, &local_usage_requirements);

      sources_decl sources_from_features;
      extract_sources(sources_from_features, local_usage_requirements);
      if (!sources_from_features.empty())
      {
         meta_targets_t new_meta_targets;
         split_sources(&simple_targets, &new_meta_targets, sources_from_features, build_request);
         instantiate_meta_targets(simple_targets, instantiated_meta_targets, 
                                  usage_requirements, new_meta_targets, 
                                  build_request, owner_for_new_targets);
      }
      else
         usage_requirements.join(local_usage_requirements);
   }

   void meta_target::compute_additional_usage_requirements(sources_decl& simple_targets,
                                                           std::vector<basic_target*>& instantiated_meta_targets,
                                                           feature_set& usage_requirements,
                                                           const sources_decl& sources_from_usage,
                                                           const feature_set& build_request,
                                                           const main_target& owner_for_new_targets) const
   {
      sources_decl ignored_simple_targets;
      meta_targets_t ignored_meta_targets;
      split_sources(&ignored_simple_targets, &ignored_meta_targets, sources_from_usage, build_request);
      std::vector<basic_target*> ignored_instantiated_meta_targets;
      feature_set* local_usage_requirements = project()->engine()->feature_registry().make_set();
      basic_meta_target::instantiate_meta_targets(ignored_meta_targets, build_request, &owner_for_new_targets, 
                                                  &ignored_instantiated_meta_targets, local_usage_requirements);

      sources_decl sources_from_uses;
      extract_uses(sources_from_uses, *local_usage_requirements);
      if (!sources_from_uses.empty())
         compute_additional_usage_requirements(simple_targets, instantiated_meta_targets, 
                                               *local_usage_requirements, sources_from_uses, 
                                               build_request, owner_for_new_targets);

      sources_decl sources_from_usage_requirements;
      extract_sources(sources_from_usage_requirements, *local_usage_requirements);
      if (!sources_from_usage_requirements.empty())
      {
         meta_targets_t meta_targets;
         split_sources(&simple_targets, &meta_targets, sources_from_usage_requirements, build_request);
         if (!meta_targets.empty())
         {
            feature_set* local_usage_requirements = project()->engine()->feature_registry().make_set();
            instantiate_meta_targets(simple_targets, instantiated_meta_targets, 
                                     *local_usage_requirements, meta_targets,
                                     build_request, owner_for_new_targets);
            sources_decl sources_from_usage_requirements;
            extract_uses(sources_from_usage_requirements, *local_usage_requirements);
            if (!sources_from_usage_requirements.empty())
               compute_additional_usage_requirements(simple_targets, instantiated_meta_targets, 
                                                     *local_usage_requirements, sources_from_usage_requirements, 
                                                     build_request, owner_for_new_targets);
            usage_requirements.join(*local_usage_requirements);
         }
      }

      usage_requirements.join(*local_usage_requirements);
   }

   void meta_target::instantiate(const main_target* owner,
                                 const feature_set& build_request,
                                 std::vector<basic_target*>* result,
                                 feature_set* usage_requirements) const
   {
      feature_set* mt_fs = build_request.clone();
      requirements().eval(build_request, mt_fs);

      feature_set* local_usage_requirements = project()->engine()->feature_registry().make_set();
      feature_set* build_request_for_dependencies = project()->engine()->feature_registry().make_set();
      build_request_for_dependencies->copy_propagated(build_request);
      build_request_for_dependencies->copy_propagated(*mt_fs);

      vector<basic_target*> instantiated_meta_targets;
      sources_decl simple_targets;
      meta_targets_t meta_targets;

      sources_decl additional_sources(owner == NULL ? sources_decl() : compute_additional_sources(*owner));
      sources_decl sources_from_requirements;
      extract_sources(sources_from_requirements, *mt_fs);

      split_sources(&simple_targets, &meta_targets, sources(), *build_request_for_dependencies); 
      split_sources(&simple_targets, &meta_targets, sources_from_requirements, *build_request_for_dependencies);
      split_sources(&simple_targets, &meta_targets, additional_sources, *build_request_for_dependencies);
      
      project()->engine()->feature_registry().add_defaults(mt_fs);
      main_target* mt = construct_main_target(mt_fs); // construct_main_target may construct main_target with different properties PCH is example
      mt_fs = mt->properties().clone(); // FIXME ref semantic required

      if (!meta_targets.empty())
         instantiate_meta_targets(simple_targets, instantiated_meta_targets, 
                                 *local_usage_requirements, meta_targets, 
                                 *build_request_for_dependencies, *mt);
      
      sources_decl sources_from_uses;
      extract_uses(sources_from_uses, *mt_fs);
      extract_uses(sources_from_uses, *local_usage_requirements);
      if (!sources_from_uses.empty())
         compute_additional_usage_requirements(simple_targets, instantiated_meta_targets, 
                                               *local_usage_requirements, sources_from_uses, 
                                               *build_request_for_dependencies, *mt);

      mt_fs->join(*local_usage_requirements);
      mt->properties(mt_fs);
      instantiate_simple_targets(simple_targets, *mt_fs, *mt, &instantiated_meta_targets);
      mt->sources(instantiated_meta_targets);
      compute_usage_requirements(*usage_requirements, *mt_fs, *local_usage_requirements);
      
      result->push_back(mt);
   }

   void meta_target::compute_usage_requirements(feature_set& result, 
                                                const feature_set& full_build_request,
                                                const feature_set& computed_usage_requirements) const
   {
      this->usage_requirements().eval(full_build_request, &result);
   }

   sources_decl meta_target::compute_additional_sources(const main_target& owner) const
   {
      return sources_decl();
   }
}