#pragma once
#include <hammer/core/meta_target.h>

namespace hammer {

class alias_meta_target : public basic_meta_target {
   public:
      alias_meta_target(hammer::project* p, const std::string& name,
                        const sources_decl& sources,
                        const requirements_decl& req,
                        const requirements_decl& usage_req);
   protected:
      void instantiate_impl(const main_target* owner,
                            const feature_set& build_request,
                            std::vector<basic_target*>* result,
                            feature_set* usage_requirements) const override;
};

}
