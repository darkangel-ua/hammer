#pragma once
#include <hammer/core/typed_meta_target.h>

namespace hammer {

class header_lib_meta_target : public typed_meta_target {
   public:
      header_lib_meta_target(hammer::project* p,
                             const std::string& name,
                             const sources_decl& sources,
                             const requirements_decl& requirements,
                             const requirements_decl& usage_requirements);
   protected:
      void compute_usage_requirements(feature_set& result,
                                      const main_target& constructed_target,
                                      const feature_set& build_request,
                                      const feature_set& computed_usage_requirements,
                                      const main_target* owner) const override;
};

}
