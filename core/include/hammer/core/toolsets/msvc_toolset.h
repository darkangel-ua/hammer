#if !defined(h_4600a2af_9f05_4199_bb10_2b42d6ca6b22)
#define h_4600a2af_9f05_4199_bb10_2b42d6ca6b22

#include <hammer/core/toolset.h>

namespace hammer {

class msvc_toolset : public toolset
{
	public:
		msvc_toolset();
		void autoconfigure(engine& e) const override;
		void configure(engine& e,
							const std::string& version) const override;

	private:
		void use_toolset_rule(invocation_context& ctx,
									 const parscore::identifier& version,
									 const parscore::identifier* path_to_vc_folder);
		void init_toolset(engine& e,
								const std::string& version_id,
								const location_t& toolset_home) const;
};

}

#endif
