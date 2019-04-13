#include <hammer/core/engine.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/testing_link_base_meta_target.h>
#include <hammer/core/testing_suite_meta_target.h>

namespace hammer {

testing_link_base_meta_target::testing_link_base_meta_target(hammer::project* p,
                                                             const std::string& name,
                                                             const requirements_decl& req,
                                                             const target_type& t)
   : typed_meta_target(p, name, req, {}, t)
{

}

void testing_link_base_meta_target::instantiate_impl(instantiation_context& ctx,
                      const main_target* owner,
                      const feature_set& build_request,
                      std::vector<basic_target*>* result,
                      feature_set* usage_requirements) const
{
   auto* suite = find_suite(get_project(), ctx);

   if (!suite || suite->common_requirements().empty())
      return typed_meta_target::instantiate_impl(ctx, owner, build_request, result, usage_requirements);

   // FIXME: we are ignoring usage requirements, so we either need to not ignor them or print error message that
   // prevents users to include @ in them
   feature_set* new_build_request = build_request.clone();
   add_common_requirements(*new_build_request, suite->common_requirements());

   return typed_meta_target::instantiate_impl(ctx, owner, *new_build_request, result, usage_requirements);
}

sources_decl
testing_link_base_meta_target::compute_additional_sources(const instantiation_context& ctx,
                                                          const main_target& owner) const
{
   auto* suite = find_suite(get_project(), ctx);
   return suite ? suite->common_sources() : sources_decl{};
}

}

