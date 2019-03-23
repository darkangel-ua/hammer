#include <hammer/core/main_target.h>
#include <hammer/core/project.h>
#include <hammer/core/instantiation_context.h>
#include <hammer/core/testing_suite_meta_target.h>
#include <hammer/core/testing_run_meta_target.h>
#include <hammer/core/testing_link_base_meta_target.h>

namespace hammer {

testing_link_base_meta_target::testing_link_base_meta_target(hammer::project* p,
                                                             const std::string& name,
                                                             const requirements_decl& req,
                                                             const target_type& t)
   : typed_meta_target(p, name, req, {}, t)
{

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

sources_decl
testing_link_base_meta_target::compute_additional_sources(const instantiation_context& ctx,
                                                          const main_target& owner) const
{
   auto* suite = find_suite(get_project(), ctx);
   return suite ? suite->common_sources() : sources_decl{};
}

}

