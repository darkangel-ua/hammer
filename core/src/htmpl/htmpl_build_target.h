#pragma once
#include <hammer/core/source_build_target.h>

namespace hammer {

class htmpl_build_target : public source_build_target
{
   public:
      htmpl_build_target(const main_target* mt,
                         const std::string& name,
                         const location_t& l,
                         const target_type* t,
                         const feature_set* f);

      const std::vector<const feature*>&
      valuable_features() const override { return valuable_features_; }

	private:
      std::vector<const feature*> valuable_features_;
};

}
