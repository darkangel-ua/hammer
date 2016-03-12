#if !defined(h_1d73b6b5_e5be_4fe6_a196_696be7eae86f)
#define h_1d73b6b5_e5be_4fe6_a196_696be7eae86f

#include <hammer/core/location.h>

namespace hammer
{
   class main_target;
   class output_location_strategy
   {
      public:
         virtual location_t compute_output_location(const main_target& mt) const = 0;
         virtual ~output_location_strategy() {}
   };
}

#endif //h_1d73b6b5_e5be_4fe6_a196_696be7eae86f
