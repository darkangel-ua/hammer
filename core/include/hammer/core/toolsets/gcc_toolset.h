#pragma once
#include <hammer/core/toolset.h>

namespace hammer {

class gcc_toolset : public toolset {
	public:
		gcc_toolset();
		void autoconfigure(engine& e) override;
		void configure(engine& e,
		               const std::string& version) override;

	private:
      struct toolset_data;

      void init_toolset(engine& e,
                        const toolset_data& td);

      YAML::Node
      make_toolset_info(const toolset_data& td);

      void use_toolset_rule(invocation_context& ctx,
		                      const parscore::identifier* version,
		                      const location_t* path_to_cxx_compiler,
		                      const location_t* path_to_linker,
		                      const location_t* path_to_ar,
		                      const parscore::identifier* c_flags,
		                      const parscore::identifier* cxx_flags,
                            const parscore::identifier* link_flags,
                            const feature_set* constraints);
};

}
