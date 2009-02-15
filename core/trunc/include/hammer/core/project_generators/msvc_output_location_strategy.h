#if !defined(h_12d6a99e_982e_4491_8a97_c8947dd8a7f4)
#define h_12d6a99e_982e_4491_8a97_c8947dd8a7f4

#include <hammer/core/output_location_strategy.h>

namespace hammer
{
   class msvc_output_location_strategy : public output_location_strategy
   {
      public:
         virtual location_t compute_output_location(const main_target& mt) const;
   };
}

#endif //h_12d6a99e_982e_4491_8a97_c8947dd8a7f4
