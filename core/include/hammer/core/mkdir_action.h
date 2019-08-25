#pragma once
#include <hammer/core/build_action.h>
#include <hammer/core/location.h>

namespace hammer {

class mkdir_action : public build_action {
   public:
      mkdir_action(const location_t& dir_to_create);
      std::string target_tag(const build_node& node,
                             const build_environment& environment) const override;
      std::vector<feature_ref> valuable_features() const override { return {}; }

   protected:
      bool execute_impl(const build_node& node,
                        const build_environment& environment) const override;

   private:
      location_t dir_to_create_;
};

}
