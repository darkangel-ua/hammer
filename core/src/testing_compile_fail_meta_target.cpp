#include <hammer/core/testing_compile_fail_meta_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/testing_main_target.h>

namespace hammer {

testing_compile_fail_meta_target::testing_compile_fail_meta_target(project* p,
                                                                   const pstring& name,
                                                                   const requirements_decl& req,
                                                                   const requirements_decl& usage_req)
   : typed_meta_target(p, name, req, usage_req, p->get_engine()->get_type_registry().get(types::TESTING_COMPILE_FAIL))
{
}

void testing_compile_fail_meta_target::instantiate_impl(const main_target* owner,
                                                        const feature_set& build_request,
                                                        std::vector<basic_target*>* result,
                                                        feature_set* usage_requirements) const
{
   // we need to convert <testing.additional-source> into <use> in build request
   // because that is how test-suite target will pass sources down if any
   feature_set* new_build_request = get_engine()->feature_registry().make_set();
   for (const feature* f : build_request) {
      if (f->name() == "testing.additional-source") {
         feature* source = get_engine()->feature_registry().create_feature("use", "");
         source->get_dependency_data() = f->get_dependency_data();
         new_build_request->join(source);
      } else {
         feature* f_copy = get_engine()->feature_registry().clone_feature(*f);
         new_build_request->join(f_copy);
      }
   }

   base::instantiate_impl(owner, *new_build_request, result, usage_requirements);
}

}
