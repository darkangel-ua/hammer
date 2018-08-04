#pragma once
#include <hammer/core/build_action.h>

namespace hammer {

class testing_success_action : public build_action {
   public:
      testing_success_action(const std::string& name,
                             const build_action_ptr& success_action);

      std::string
		target_tag(const build_node& node,
		           const build_environment& environment) const override;

		std::vector<const feature*>
		valuable_features() const override { return success_action_->valuable_features(); }

	protected:
		bool execute_impl(const build_node& node,
		                  const build_environment& environment) const override;
   private:
      build_action_ptr success_action_;
};

}
