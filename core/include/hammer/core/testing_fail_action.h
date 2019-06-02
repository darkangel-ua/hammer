#pragma once
#include <hammer/core/build_action.h>
#include <hammer/core/target_type.h>
#include <hammer/core/type_tag.h>

namespace hammer {

class engine;

class testing_fail_action : public build_action {
   public:
      testing_fail_action(engine& e,
                          const std::string& name,
		                    build_action_ptr failing_action);

		std::string
      target_tag(const build_node& node,
		           const build_environment& environment) const override;

		std::vector<const feature*>
      valuable_features() const override;

	protected:
      bool execute_impl(const build_node& node,
                        const build_environment& environment) const override;

   private:
      build_action_ptr failing_action_;
};

}
