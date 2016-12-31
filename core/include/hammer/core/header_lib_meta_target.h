#if !defined(h_5dfc9743_f59c_458d_9f35_b8d36006bf2e)
#define h_5dfc9743_f59c_458d_9f35_b8d36006bf2e

#include <hammer/core/meta_target.h>

namespace hammer{

class header_lib_meta_target : public meta_target
{
   public:
      header_lib_meta_target(hammer::project* p,
                             const pstring& name,
                             const requirements_decl& requirements,
                             const requirements_decl& usage_requirements);
   protected:
      void instantiate_impl(const main_target* owner,
                            const feature_set& build_request,
                            std::vector<basic_target*>* result,
                            feature_set* usage_requirements) const override;
      main_target*
      construct_main_target(const main_target* owner,
                            const feature_set* properties) const override;
};

}

#endif
