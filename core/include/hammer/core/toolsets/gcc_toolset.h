#if !defined(h_dbbb68bd_8832_4d6c_aa77_baeb28bdbc79)
#define h_dbbb68bd_8832_4d6c_aa77_baeb28bdbc79

#include <hammer/core/toolset.h>

namespace hammer {

class gcc_toolset : public toolset
{
	public:
		gcc_toolset();
		void autoconfigure(engine& e) const override;
		void configure(engine& e,
		               const std::string& version) const override;

	private:
		void use_toolset_rule(invocation_context& ctx,
		                      const parscore::identifier* version,
		                      const location_t* path_to_cxx_compiler,
		                      const location_t* path_to_linker,
		                      const location_t* path_to_ar,
		                      const parscore::identifier* c_flags,
		                      const parscore::identifier* cxx_flags,
                            const parscore::identifier* link_flags);
};

}

#endif
