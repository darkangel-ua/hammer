#ifndef HAMMER_CORE_TESTING_COMPILE_FAIL_META_TARGET_H
#define HAMMER_CORE_TESTING_COMPILE_FAIL_META_TARGET_H

#include <hammer/core/typed_meta_target.h>

namespace hammer {

class testing_compile_fail_meta_target : public typed_meta_target
{
   public:
      typedef typed_meta_target base;
      testing_compile_fail_meta_target(hammer::project* p,
                                       const pstring& name,
                                       const requirements_decl& req,
                                       const requirements_decl& usage_req);
   protected:
      void instantiate_impl(const main_target* owner,
                            const feature_set& build_request,
                            std::vector<basic_target*>* result,
                            feature_set* usage_requirements) const override;
};

}
#endif // TESTING_COMPILE_FAIL_META_TARGET_H
