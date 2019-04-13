#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/instantiation_context.h>
#include <hammer/core/testing_run_meta_target.h>
#include <hammer/core/testing_suite_meta_target.h>

namespace hammer {

testing_suite_meta_target::testing_suite_meta_target(hammer::project& p,
                                                     const std::string& name,
                                                     const sources_decl& sources,
                                                     const sources_decl& common_sources,
                                                     const requirements_decl& common_requirements)
   : typed_meta_target(&p, name, {}, {}, p.get_engine().get_type_registry().get(types::TESTING_SUITE)),
     common_sources_(common_sources),
     common_requirements_(common_requirements)
{
   this->sources(sources);
}

void testing_suite_meta_target::common_sources(const sources_decl& s) {
   common_sources_ = s;
}

const testing_suite_meta_target*
find_suite(const project &p,
           const instantiation_context& ctx) {
   if (ctx.get_stack().size() <= 1)
      return nullptr;

   auto i = ctx.get_stack().rbegin();
   auto mt1 = *(++i);

   if (mt1->get_project() == p && dynamic_cast<const testing_suite_meta_target*>(mt1))
      return static_cast<const testing_suite_meta_target*>(mt1);

   if (ctx.get_stack().size() >=3) {
      auto mt2 = *(++i);
      if (mt1->get_project() == p && mt2->get_project() == p &&
          dynamic_cast<const testing_run_meta_target*>(mt1) &&
          dynamic_cast<const testing_suite_meta_target*>(mt2))
      {
         return static_cast<const testing_suite_meta_target*>(mt2);
      }
   }

   return nullptr;
}

void add_common_requirements(feature_set& build_request,
                             const requirements_decl& req) {
   feature_set* additional_requirements = build_request.owner().make_set();
   req.eval(build_request, additional_requirements);
   build_request.join(*additional_requirements);
}

}
