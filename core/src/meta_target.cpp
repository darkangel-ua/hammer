#include "stdafx.h"
#include <boost/ref.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/optional.hpp>
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

namespace {

struct meta_targets_get_first
{
   typedef const basic_meta_target* result_type;
   const basic_meta_target* operator () (const meta_targets_t::value_type& v) const { return v.first; }
};

using namespace boost::multi_index;
typedef meta_targets_t::value_type one_meta_target_t;
typedef multi_index_container<one_meta_target_t,
                              indexed_by<hashed_unique<meta_targets_get_first
                                                      >,
                                         sequenced<>
                                        >
                             > deduplicator_t;

void remove_duplicates(deduplicator_t& deduplicator,
                       meta_targets_t& targets)
{
   const auto old_size = deduplicator.get<1>().size();
   boost::optional<deduplicator_t::nth_index_const_iterator<1>::type> last_good_source_iterator;
   // magic construct to take iterator on last element
   if (!deduplicator.empty())
      last_good_source_iterator = (++deduplicator.get<1>().rbegin()).base();

   for (const auto& t : targets)
      deduplicator.get<1>().push_back(t);

   // if we don't have dups - just return
   if (old_size + targets.size() == deduplicator.get<1>().size())
      return;

   // else - construct new list without dups and replace existing
   auto from = !last_good_source_iterator ? deduplicator.get<1>().begin()
                                          : ++(*last_good_source_iterator);
   meta_targets_t targets_without_dups(from, deduplicator.get<1>().end());
   targets.swap(targets_without_dups);
}

}

   meta_target::meta_target(hammer::project* p,
                            const std::string& name,
                            const requirements_decl& props, 
                            const requirements_decl& usage_req) 
                           : basic_meta_target(p, name, props, usage_req)
   {
   }

   static
   void instantiate_meta_targets(instantiation_context& ctx,
                                 sources_decl& simple_targets,
                                 std::vector<basic_target*>& instantiated_meta_targets,
                                 deduplicator_t& sources_deduplicator,
                                 sources_decl& additional_sources,
                                 feature_set& usage_requirements,
                                 const basic_meta_target& this_,
                                 const meta_targets_t& meta_targets,
                                 const feature_set& build_request,
                                 const main_target& owner_for_new_targets)
   {
      feature_set& local_usage_requirements = *this_.get_engine().feature_registry().make_set();
      instantiate_meta_targets(ctx, meta_targets, &owner_for_new_targets, &instantiated_meta_targets, &local_usage_requirements);

      sources_decl sources_from_features;
      extract_sources(sources_from_features, local_usage_requirements, this_);
      usage_requirements.join(local_usage_requirements);
      if (!sources_from_features.empty())
      {
         additional_sources.insert(sources_from_features);

         meta_targets_t new_meta_targets;
         this_.split_sources(&simple_targets, &new_meta_targets, sources_from_features, build_request);
         remove_duplicates(sources_deduplicator, new_meta_targets);
         instantiate_meta_targets(ctx, simple_targets, instantiated_meta_targets, sources_deduplicator, additional_sources,
                                  usage_requirements, this_, new_meta_targets,
                                  build_request, owner_for_new_targets);
      }
   }

   // instantiate targets that we found in use feature and
   // add additional usage requirements to main target usage requirements
   void compute_additional_usage_requirements(instantiation_context& ctx,
                                              sources_decl& simple_targets,
                                              std::vector<basic_target*>& instantiated_meta_targets,
                                              deduplicator_t& sources_deduplicator,
                                              feature_set& usage_requirements,
                                              const basic_meta_target& this_,
                                              const sources_decl& sources_from_usage,
                                              const feature_set& build_request,
                                              const main_target& owner_for_new_targets)
   {
      sources_decl ignored_simple_targets;
      sources_decl ignored_additional_sources;
      meta_targets_t ignored_meta_targets;
      this_.split_sources(&ignored_simple_targets, &ignored_meta_targets, sources_from_usage, build_request);
      remove_duplicates(sources_deduplicator, ignored_meta_targets);
      std::vector<basic_target*> ignored_instantiated_meta_targets;
      feature_set* local_usage_requirements = this_.get_engine().feature_registry().make_set();
      instantiate_meta_targets(ctx, ignored_meta_targets, &owner_for_new_targets,
                               &ignored_instantiated_meta_targets, local_usage_requirements);

      sources_decl sources_from_uses;
      extract_uses(sources_from_uses, *local_usage_requirements, this_);
      if (!sources_from_uses.empty())
         compute_additional_usage_requirements(ctx, simple_targets, instantiated_meta_targets, sources_deduplicator,
                                               *local_usage_requirements, this_, sources_from_uses,
                                               build_request, owner_for_new_targets);

      sources_decl sources_from_usage_requirements;
      extract_sources(sources_from_usage_requirements, *local_usage_requirements, this_);
      if (!sources_from_usage_requirements.empty())
      {
         meta_targets_t meta_targets;
         this_.split_sources(&simple_targets, &meta_targets, sources_from_usage_requirements, build_request);
         remove_duplicates(sources_deduplicator, meta_targets);
         if (!meta_targets.empty())
         {
            feature_set* local_usage_requirements = this_.get_engine().feature_registry().make_set();
            instantiate_meta_targets(ctx, simple_targets, instantiated_meta_targets, sources_deduplicator, ignored_additional_sources,
                                     *local_usage_requirements, this_, meta_targets,
                                     build_request, owner_for_new_targets);
            sources_decl sources_from_usage_requirements;
            extract_uses(sources_from_usage_requirements, *local_usage_requirements, this_);
            if (!sources_from_usage_requirements.empty())
               compute_additional_usage_requirements(ctx, simple_targets, instantiated_meta_targets, sources_deduplicator,
                                                     *local_usage_requirements, this_, sources_from_usage_requirements,
                                                     build_request, owner_for_new_targets);
            usage_requirements.join(*local_usage_requirements);
         }
      }

      usage_requirements.join(*local_usage_requirements);
   }

   static
   void transfer_public_sources(feature_set& dest,
                                const sources_decl& sources,
                                const feature_set& build_request,
                                feature_registry& fr,
                                const basic_meta_target& relative_to_target)
   {
      // when transferring public sources we should make <use> with current build request applied
      // because this is the only way to produce correct usage requirements in dependent targets
      // same as in compute_usage_requirements
      for (const source_decl& source : sources) {
         if (source.is_public()) {
            source_decl sd_copy = source;
            sd_copy.build_request_join(build_request);
            dest.join(fr.create_feature("use", sd_copy));
         }
      }
   }
   
   void meta_target::instantiate_impl(instantiation_context& ctx,
                                      const main_target* owner,
                                      sources_decl sources,
                                      const feature_set& build_request,
                                      std::vector<basic_target*>* result,
                                      feature_set* usage_requirements) const {
      apply_project_dependencies(sources, *this);

      feature_set* mt_fs = build_request.clone();
      feature_set* public_requirements = get_engine().feature_registry().make_set();
      requirements().eval(build_request, mt_fs, public_requirements);
      apply_project_dependencies(*public_requirements, *this);
      usage_requirements->join(*public_requirements);

      feature_set* build_request_for_dependencies = get_engine().feature_registry().make_set();
      build_request_for_dependencies->copy_propagated(build_request);
      build_request_for_dependencies->copy_propagated(*mt_fs);

      vector<basic_target*> instantiated_meta_targets;
      vector<basic_target*> instantiated_dependency_meta_targets;
      sources_decl simple_targets;
      meta_targets_t meta_targets;
      meta_targets_t dependency_meta_targets;

      sources_decl additional_sources(owner ? compute_additional_sources(ctx, *owner) : sources_decl());
      apply_project_dependencies(additional_sources, *this);

      sources_decl sources_from_requirements;
      sources_decl sources_from_features;
      sources_decl dependencies_from_requierements;
      extract_sources(sources_from_requirements, *mt_fs, *this);
      apply_project_dependencies(sources_from_requirements, *this);
      extract_dependencies(dependencies_from_requierements, *mt_fs, *this);
      apply_project_dependencies(dependencies_from_requierements, *this);

      split_sources(&simple_targets, &meta_targets, sources, *build_request_for_dependencies);
      split_sources(&simple_targets, &meta_targets, sources_from_requirements, *build_request_for_dependencies);
      split_sources(&simple_targets, &meta_targets, additional_sources, *build_request_for_dependencies);
      split_sources(&simple_targets, &dependency_meta_targets, dependencies_from_requierements, *build_request_for_dependencies);

      deduplicator_t sources_deduplicator;
      remove_duplicates(sources_deduplicator, meta_targets);

      get_project().feature_registry().add_defaults(*mt_fs);

      main_target* mt = construct_main_target(owner, mt_fs); // construct_main_target may construct main_target with different properties PCH is example
      mt_fs = mt->properties().clone(); // FIXME ref semantic required

      feature_set* local_usage_requirements = get_engine().feature_registry().make_set();
      if (!meta_targets.empty())
         instantiate_meta_targets(ctx, simple_targets, instantiated_meta_targets, sources_deduplicator, sources_from_features,
                                  *local_usage_requirements, *this, meta_targets,
                                  *build_request_for_dependencies, *mt);

      sources_decl dependencies_from_instantiations;
      extract_dependencies(dependencies_from_instantiations, *local_usage_requirements, *this);
      split_sources(&simple_targets, &dependency_meta_targets, dependencies_from_instantiations, *build_request_for_dependencies);

      deduplicator_t dependency_sources_deduplicator;
      remove_duplicates(dependency_sources_deduplicator, dependency_meta_targets);
      feature_set* ignored_dependencies_usage_requirements = get_engine().feature_registry().make_set();
      if (!dependency_meta_targets.empty())
         instantiate_meta_targets(ctx, simple_targets, instantiated_dependency_meta_targets, dependency_sources_deduplicator, sources_from_features,
                                 *ignored_dependencies_usage_requirements, *this, dependency_meta_targets,
                                 *build_request_for_dependencies, *mt);

      sources_decl sources_from_uses;
      extract_uses(sources_from_uses, *mt_fs, *this);
      apply_project_dependencies(sources_from_uses, *this);
      extract_uses(sources_from_uses, *local_usage_requirements, *this);
      if (!sources_from_uses.empty()) {
         deduplicator_t use_sources_deduplicator;
         compute_additional_usage_requirements(ctx, simple_targets, instantiated_meta_targets, use_sources_deduplicator,
                                               *local_usage_requirements, *this, sources_from_uses,
                                               *build_request_for_dependencies, *mt);
      }

      mt_fs->join(*local_usage_requirements);
      mt->properties(mt_fs);
      instantiate_simple_targets(simple_targets, *mt_fs, *mt, &instantiated_meta_targets);
      mt->sources(instantiated_meta_targets);
      mt->dependencies(instantiated_dependency_meta_targets);

      sources_decl all_sources = sources;
      all_sources.insert(sources_from_requirements);
      all_sources.insert(sources_from_features);
      transfer_public_sources(*usage_requirements, all_sources, *build_request_for_dependencies, get_engine().feature_registry(), *this);
      compute_usage_requirements(*usage_requirements, *mt, *build_request_for_dependencies, *local_usage_requirements, owner);

      // we need to transform references in dependency features to local meta-targets to './/foo' form
      adjust_dependency_features_sources(*usage_requirements);

      result->push_back(mt);
   }

   void meta_target::instantiate_impl(instantiation_context& ctx,
                                      const main_target* owner,
                                      const feature_set& build_request_param,
                                      std::vector<basic_target*>* result,
                                      feature_set* usage_requirements) const
   {
      instantiate_impl(ctx, owner, sources(), build_request_param, result, usage_requirements);
   }

   void meta_target::compute_usage_requirements(feature_set& result, 
                                                const main_target& constructed_target,
                                                const feature_set& build_request,
                                                const feature_set& computed_usage_requirements,
                                                const main_target* owner) const
   {
      feature_set& tmp = *get_engine().feature_registry().make_set();
      this->usage_requirements().eval(constructed_target.properties(), &tmp);
      apply_project_dependencies(tmp, *this);

      // we need to transform references in dependency features to local meta-targets to './/foo' form
      adjust_dependency_features_sources(tmp);

      // when transferring public sources we should make <use> with current build request applied
      // because this is the only way to produce correct usage requirements in dependent targets
      // same as in transfer_public_sources
      apply_build_request(tmp, build_request);

      result.join(tmp);
   }

   sources_decl
   meta_target::compute_additional_sources(const instantiation_context&,
                                           const main_target&) const
   {
      return {};
   }

   static
   void apply_project_dependencies(source_decl& s,
                                   const project::dependencies_t& dependencies) {
      if (!s.is_meta_target())
         return;

      for (auto& d : dependencies) {
         if (boost::regex_match(s.target_path(), d.target_ref_mask_))
            s.build_request_join(*d.properties_);
      }
   }

   void apply_project_dependencies(sources_decl& sources,
                                   const basic_meta_target& sources_owner) {
      const auto& dependencies = sources_owner.get_project().dependencies();
      for (auto& s : sources)
         apply_project_dependencies(s, dependencies);
   }

   sources_decl
   apply_project_dependencies(sources_decl&& sources,
                              const basic_meta_target& sources_owner) {
      auto s = std::move(sources);
      apply_project_dependencies(s, sources_owner);
      return std::move(s);
   }

   void apply_project_dependencies(feature_set& properties,
                                   const basic_meta_target& sources_owner) {
      const auto& dependencies = sources_owner.get_project().dependencies();
      for (auto i = properties.begin(), last = properties.end(); i != last; ++i) {
         const feature& f = **i;
         if (!f.attributes().dependency)
            continue;

         auto new_sd = f.get_dependency_data().source_;
         apply_project_dependencies(new_sd, dependencies);
         properties.replace(i, properties.owner().create_feature(f.name(), new_sd));
      }
   }
}
