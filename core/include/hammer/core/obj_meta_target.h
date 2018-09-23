#pragma once
#include <hammer/core/typed_meta_target.h>

namespace hammer {

class obj_meta_target : public typed_meta_target {
   public:
      obj_meta_target(hammer::project* p,
                      const std::string& name,
                      const requirements_decl& req,
                      const requirements_decl& usage_req);
   protected:
      bool is_cachable(const main_target* owner) const override { return false; }
};

}
