#pragma once
#include <hammer/core/meta_target.h>

namespace hammer {

class lib_meta_target : public meta_target {
   public:
      lib_meta_target(hammer::project* p,
                      const std::string& name,
                      const requirements_decl& req,
                      const requirements_decl& usage_req);

   protected:
      main_target*
      construct_main_target(const main_target* owner,
                            const feature_set* properties) const override;
};

}
