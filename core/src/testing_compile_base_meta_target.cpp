#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/testing_suite_meta_target.h>
#include <hammer/core/testing_compile_base_meta_target.h>

namespace hammer {

testing_compile_base_meta_target::testing_compile_base_meta_target(hammer::project* p,
                                                                   const std::string& name,
                                                                   const requirements_decl& req,
                                                                   const target_type& t)
   : testing_base_meta_target{p, name, req, t}
{

}

void testing_compile_base_meta_target::instantiate_impl(instantiation_context& ctx,
                                                        const main_target* owner,
                                                        const feature_set& build_request,
                                                        std::vector<basic_target*>* result,
                                                        feature_set* usage_requirements) const
{
   auto* suite = find_suite(get_project(), ctx);
   if (!suite)
      return typed_meta_target::instantiate_impl(ctx, owner, build_request, result, usage_requirements);

   feature_set& new_build_request = *build_request.clone();

   // we need to instantiate all sources found in testing.suite common-sources
   // and than attach their usage-requirements to this build-request
   // we instantiate because sources might have <sources> in usage-requirements and this will add
   // sources to this target and than generator will fail to find transformation because it doesn't know
   // how to do CPP + SHARED_LIB -> TESTING_COMPILE_SUCCESSFUL
   feature_set& common_usage_requirements = *get_engine().feature_registry().make_set();

   sources_decl simple_targets;
   meta_targets_t meta_targets;
   std::vector<basic_target*> ignored_targets;
   split_sources(&simple_targets, &meta_targets, suite->common_sources(), build_request);
   instantiate_meta_targets(ctx, meta_targets, nullptr, &ignored_targets, &common_usage_requirements);

   // now we need to remove any <source>/<dependency> in calculated usage requirements
   common_usage_requirements.erase_all("source");
   common_usage_requirements.erase_all("dependency");

   // and finally join build request and common usage requirements
   new_build_request.join(common_usage_requirements);

   if (!suite->common_requirements().empty())
      add_common_requirements(new_build_request, suite->common_requirements());

   typed_meta_target::instantiate_impl(ctx, owner, new_build_request, result, usage_requirements);
}

}

