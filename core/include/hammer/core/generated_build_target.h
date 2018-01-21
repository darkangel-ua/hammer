#pragma once
#include <hammer/core/file_build_target.h>

namespace hammer {

class generated_build_target : public file_build_target
{
	public:
		generated_build_target(const main_target* mt,
		                       const std::string& n,
		                       const target_type* t,
		                       const feature_set* f,
		                       const location_t* l = nullptr);
		const location_t& location() const override { return location_; }
		void clean(const build_environment& environment) const override;

	private:
		const location_t location_;
};

}
