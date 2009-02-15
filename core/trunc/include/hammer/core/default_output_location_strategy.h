#if !defined(h_3f4d4eb2_0937_473e_9959_58d62b583039)
#define h_3f4d4eb2_0937_473e_9959_58d62b583039

#include "output_location_strategy.h"

namespace hammer
{
   class default_output_location_strategy : public output_location_strategy
   {
      public:
         virtual location_t compute_output_location(const main_target& mt) const;
   };
}

#endif //h_3f4d4eb2_0937_473e_9959_58d62b583039
