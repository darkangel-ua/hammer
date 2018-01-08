#pragma once
#include <hammer/core/basic_build_target.h>

namespace hammer {

class file_build_target : public basic_build_target
{
	public:
		file_build_target(const main_target* mt,
		                  const std::string& name,
		                  const target_type* t,
		                  const feature_set* f)
		   : basic_build_target(mt, name, t, f)
		{
		}

	protected:
		void timestamp_info_impl() const override;
};

}
