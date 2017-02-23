#if !defined(h_5dfc9743_f59c_458d_9f35_b8d36006bf2e)
#define h_5dfc9743_f59c_458d_9f35_b8d36006bf2e

#include <hammer/core/basic_meta_target.h>

namespace hammer{

class header_lib_meta_target : public basic_meta_target
{
   public:
      header_lib_meta_target(hammer::project* p,
                             const std::string& name,
                             const requirements_decl& requirements,
                             const requirements_decl& usage_requirements);
   protected:
      void instantiate_impl(const main_target* owner,
                            const feature_set& build_request,
                            std::vector<basic_target*>* result,
                            feature_set* usage_requirements) const override;
};

}

#endif
