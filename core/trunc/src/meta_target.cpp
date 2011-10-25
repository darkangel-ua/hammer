#include "stdafx.h"
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/source_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature.h>
#include <hammer/core/location.h>
#include <hammer/core/requirements_decl.h>

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
      feature_set& local_usage_requirements = *get_engine()->feature_registry().make_set();
      basic_meta_target::instantiate_meta_targets(meta_targets, build_request, &owner_for_new_targets, 
                                                  &instantiated_meta_targets, &local_usage_requirements);

      sources_decl sources_from_features;
      extract_sources(sources_from_features, local_usage_requirements);
      usage_requirements.join(local_usage_requirements);
      if (!sources_from_features.empty())
      {
         meta_targets_t new_meta_targets;
         split_sources(&simple_targets, &new_meta_targets, sources_from_features, build_request);
         instantiate_meta_targets(simple_targets, instantiated_meta_targets, 
                                  usage_requirements, new_meta_targets, 
                                  build_request, owner_for_new_targets);
      }
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
      feature_set* local_usage_requirements = get_engine()->feature_registry().make_set();
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
            feature_set* local_usage_requirements = get_engine()->feature_registry().make_set();
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

   static void transfer_public_sources(feature_set& dest,
                                       const sources_decl& sources, 
                                       feature_registry& fr)
   {
      for(sources_decl::const_iterator i = sources.begin(), last = sources.end(); i != last; ++i)
         if (i->is_public())
         {
            feature* f = fr.create_feature("use", "");
            f->get_dependency_data().source_ = *i;
            dest.join(f);
         }
   }
   
   void meta_target::instantiate_impl(const main_target* owner,
                                      const feature_set& build_request_param,
                                      std::vector<basic_target*>* result,
                                      feature_set* usage_requirements) const
   {
      const feature_set& build_request = build_request_param.has_undefined_features() 
                                              ? resolve_undefined_features(build_request_param)
                                              : build_request_param;

      feature_set* mt_fs = build_request.clone();
      requirements().eval(build_request, mt_fs, usage_requirements);

      feature_set* local_usage_requirements = get_engine()->feature_registry().make_set();
      feature_set* build_request_for_dependencies = get_engine()->feature_registry().make_set();
      build_request_for_dependencies->copy_propagated(build_request);
      build_request_for_dependencies->copy_propagated(*mt_fs);

      vector<basic_target*> instantiated_meta_targets;
      vector<basic_target*> instantiated_dependency_meta_targets;
      sources_decl simple_targets;
      meta_targets_t meta_targets;
      meta_targets_t dependency_meta_targets;

      sources_decl additional_sources(owner == NULL ? sources_decl() : compute_additional_sources(*owner));
      sources_decl sources_from_requirements;
      sources_decl dependencies_from_requierements;
      extract_sources(sources_from_requirements, *mt_fs);
      extract_dependencies(dependencies_from_requierements, *mt_fs);

      split_sources(&simple_targets, &meta_targets, sources(), *build_request_for_dependencies); 
      split_sources(&simple_targets, &meta_targets, sources_from_requirements, *build_request_for_dependencies);
      split_sources(&simple_targets, &meta_targets, additional_sources, *build_request_for_dependencies);
      split_sources(&simple_targets, &dependency_meta_targets, dependencies_from_requierements, *build_request_for_dependencies);
      
      get_engine()->feature_registry().add_defaults(mt_fs);
      get_project()->local_feature_registry().add_defaults(mt_fs);

      main_target* mt = construct_main_target(owner, mt_fs); // construct_main_target may construct main_target with different properties PCH is example
      mt_fs = mt->properties().clone(); // FIXME ref semantic required

      if (!meta_targets.empty())
         instantiate_meta_targets(simple_targets, instantiated_meta_targets, 
                                  *local_usage_requirements, meta_targets, 
                                  *build_request_for_dependencies, *mt);
      
      sources_decl dependencies_from_instantiations;
      extract_dependencies(dependencies_from_instantiations, *local_usage_requirements);
      split_sources(&simple_targets, &dependency_meta_targets, dependencies_from_instantiations, *build_request_for_dependencies);

      feature_set* ignored_dependencies_usage_requirements = get_engine()->feature_registry().make_set();      
      if (!dependency_meta_targets.empty())
         instantiate_meta_targets(simple_targets, instantiated_dependency_meta_targets, 
                                 *ignored_dependencies_usage_requirements, dependency_meta_targets,
                                 *build_request_for_dependencies, *mt);


      // calculate src_dependencies
      sources_decl src_dependencies;
      sources_decl src_dependencies_simple_targets;
      vector<basic_target*> instantiated_src_dependency_meta_targets;
      meta_targets_t src_dependency_meta_targets;
      extract_src_dependencies(src_dependencies, *local_usage_requirements);
      split_sources(&src_dependencies_simple_targets, &src_dependency_meta_targets, src_dependencies, *build_request_for_dependencies); 
      if (!src_dependencies.empty())
         instantiate_meta_targets(src_dependencies_simple_targets, instantiated_src_dependency_meta_targets, 
                                 *ignored_dependencies_usage_requirements, src_dependency_meta_targets,
                                 *build_request_for_dependencies, *mt);
      mt->src_dependencies(instantiated_src_dependency_meta_targets);
      
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
      mt->dependencies(instantiated_dependency_meta_targets);
      
      transfer_public_sources(*usage_requirements, sources(), get_engine()->feature_registry());
      compute_usage_requirements(*usage_requirements, *mt, *mt_fs, *local_usage_requirements, owner);
      
      result->push_back(mt);
   }

   void meta_target::compute_usage_requirements(feature_set& result, 
                                                const main_target& constructed_target,
                                                const feature_set& full_build_request,
                                                const feature_set& computed_usage_requirements,
                                                const main_target* owner) const
   {
      this->usage_requirements().eval(full_build_request, &result);
   }

   sources_decl meta_target::compute_additional_sources(const main_target& owner) const
   {
      return sources_decl();
   }
}
