#pragma once
#include <hammer/core/file_build_target.h>

namespace hammer {

class source_build_target : public file_build_target
{
   public:
      source_build_target(const main_target* mt,
		                    const std::string& name,
		                    const location_t& l,
		                    const target_type* t,
		                    const feature_set* f)
		   : file_build_target(mt, name, t, f),
		     location_(l)
		{}

		const location_t& location() const override { return location_; }
		const std::string& hash() const override;
		bool clean(const build_environment& environment) const override { return false; }

	private:
		const location_t location_;
};

}
