#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/testing_link_base_meta_target.h>

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
   // we need to convert <testing.additional-source> into <source> in build request
   // because that is how test-suite target will pass sources down if any
   feature_set* new_build_request = get_engine().feature_registry().make_set();
   for (const feature* f : build_request) {
      if (f->name() == "testing.additional-source") {
         feature* source = get_engine().feature_registry().create_feature("source", "");
         source->set_dependency_data(f->get_dependency_data().source_, f->get_path_data().project_);
         new_build_request->join(source);
      } else {
         feature* f_copy = get_engine().feature_registry().clone_feature(*f);
         new_build_request->join(f_copy);
      }
   }

   typed_meta_target::instantiate_impl(ctx, owner, *new_build_request, result, usage_requirements);
}

}

