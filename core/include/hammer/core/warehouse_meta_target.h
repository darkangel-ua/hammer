#pragma once
#include <hammer/core/meta_target.h>

namespace hammer {

// FIXME: we should inherit from basic_meta_target but
// main_target take meta_target istead of basic_meta_target
class warehouse_meta_target : public meta_target {
   public:
      warehouse_meta_target(project& p,
                            const std::string& name,
                            const std::string& version);
      ~warehouse_meta_target();

   protected:
      void instantiate_impl(instantiation_context& ctx,
                            const main_target* owner,
                            const feature_set& build_request,
                            std::vector<basic_target*>* result,
                            feature_set* usage_requirements) const override;
      main_target*
      construct_main_target(const main_target* owner,
                            const feature_set* properties) const override;
};

}
