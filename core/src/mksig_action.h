#pragma once
#include <hammer/core/build_action.h>
#include <hammer/core/location.h>

namespace hammer {

class mksig_action : public build_action {
	public:
		mksig_action();

		std::string
		target_tag(const build_node& node,
		           const build_environment& environment) const override;

		std::vector<feature_ref>
		valuable_features() const override { return {}; }

	protected:
		bool execute_impl(const build_node& node,
		                  const build_environment& environment) const override;
};

}
