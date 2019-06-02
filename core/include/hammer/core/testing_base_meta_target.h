#pragma once
#include <hammer/core/typed_meta_target.h>

namespace hammer {

class testing_base_meta_target : public typed_meta_target {
   public:
      testing_base_meta_target(hammer::project* p,
                               const std::string& name,
                               const requirements_decl& req,
                               const target_type& t);
   protected:
      main_target*
      construct_main_target(const main_target* owner,
                            const feature_set* properties) const override;
};

}
