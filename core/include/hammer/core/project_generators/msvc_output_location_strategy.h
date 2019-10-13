#pragma once
#include <hammer/core/output_location_strategy.h>

namespace hammer {

class msvc_output_location_strategy : public output_location_strategy {
   public:
      location_t compute_output_location(const main_target& mt) const override;
};

}
