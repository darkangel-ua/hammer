#pragma once
#include <hammer/core/generated_build_target.h>

namespace hammer {

class build_node;

class signature_build_target : public generated_build_target {
   public:
		// for composite targets, write properties + sources
		// sources needs because we need to rebuild composite target when we REMOVE some simple source
		signature_build_target(const main_target* mt,
		                       const build_node& node);

		// for simple targets, write only properties
		signature_build_target(const main_target* mt,
		                       const feature_set& build_request,
		                       const location_t& l);
		const std::string& content() const { return content_; }

   protected:
      void timestamp_info_impl() const override;

	private:
		const std::string content_;
};

}
