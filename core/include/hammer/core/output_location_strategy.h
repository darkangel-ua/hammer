#pragma once
#include <hammer/core/location.h>

namespace hammer {

class main_target;

class output_location_strategy {
   public:
      virtual location_t compute_output_location(const main_target& mt) const = 0;
      virtual ~output_location_strategy() {}
};

}
