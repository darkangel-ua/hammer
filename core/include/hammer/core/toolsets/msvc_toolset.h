#pragma once
#include <hammer/core/toolset.h>

namespace hammer {

class msvc_toolset : public toolset {
	public:
		msvc_toolset();
		void autoconfigure(engine& e) override;
		void configure(engine& e,
							const std::string& version) override;

	private:
		void use_toolset_rule(invocation_context& ctx,
									 const parscore::identifier& version,
									 const parscore::identifier* path_to_vc_folder);
		void init_toolset(engine& e,
								const std::string& version_id,
								const location_t& toolset_home);
};

}
