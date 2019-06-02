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

   // we need to add <use> feature for sources found in testing.suite common-sources
   feature_set& new_build_request = *build_request.clone();
   for (const source_decl s : suite->common_sources()) {
      feature* source = get_engine().feature_registry().create_feature("use", "");
      source->set_dependency_data(s, &get_project());
      new_build_request.join(source);
   }

   if (!suite->common_requirements().empty())
      add_common_requirements(new_build_request, suite->common_requirements());

   typed_meta_target::instantiate_impl(ctx, owner, new_build_request, result, usage_requirements);
}

}

