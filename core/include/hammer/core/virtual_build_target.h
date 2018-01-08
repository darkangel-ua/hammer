#pragma once
#include <hammer/core/basic_build_target.h>

namespace hammer {

class virtual_build_target : public basic_build_target
{
   public:
		virtual_build_target(const main_target* mt,
		                     const std::string& name,
		                     const target_type* t,
		                     const feature_set* f,
		                     const bool exists);
		const location_t& location() const override;
		void clean(const build_environment& environment) const override {}

   protected:
      void timestamp_info_impl() const override;
};

}
