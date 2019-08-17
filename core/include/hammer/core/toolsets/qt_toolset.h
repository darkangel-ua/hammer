#pragma once
#include <hammer/core/toolset.h>

namespace hammer {

class qt_toolset : public toolset {
	public:
    	qt_toolset();
    	void autoconfigure(engine& e) override;
		void configure(engine& e,
		               const std::string& version) override;

	protected:
		void use_toolset_rule(invocation_context& ctx,
									 const parscore::identifier& version,
									 const location_t& root_folder);
		void init_toolset(engine& e,
								const std::string& version_id,
								const location_t& toolset_home) const;
};

class type_tag;

extern const type_tag qt_mockable;
extern const type_tag qt_ui;
extern const type_tag qt_rc;

}
