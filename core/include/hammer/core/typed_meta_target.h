#pragma once
#include <hammer/core/meta_target.h>

namespace hammer {

class target_type;

class typed_meta_target : public meta_target {
   public:
      typed_meta_target(hammer::project* p,
                        const std::string& name,
                        const requirements_decl& req,
                        const requirements_decl& usage_req,
                        const target_type& t) :
         meta_target(p, name, req, usage_req), type_(&t)
      {
      }

      const target_type& type() const { return *type_; }

   protected:
      main_target*
      construct_main_target(const main_target* owner,
                            const feature_set* properties) const override;

  private:
     const target_type* type_;
};

}
