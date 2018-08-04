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
		                    const build_action_ptr& fail_action,
                          const type_tag& output_target_type);

		std::string
      target_tag(const build_node& node,
		           const build_environment& environment) const override;

		std::vector<const feature*>
      valuable_features() const override;

	protected:
      bool execute_impl(const build_node& node,
                        const build_environment& environment) const override;

   private:
      build_action_ptr fail_action_;
      const target_type& output_target_type_;
};

}
