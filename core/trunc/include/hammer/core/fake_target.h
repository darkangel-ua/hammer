#if !defined(h_4b03135f_1e7a_4294_b464_d7802b6e8628)
#define h_4b03135f_1e7a_4294_b464_d7802b6e8628

#include "basic_target.h"

namespace hammer
{
   class fake_target : public basic_target
   {
      public:
         fake_target(const main_target* mt, const pstring& name,
                     const target_type* t, const feature_set* f);
         virtual const location_t& location() const;
         virtual build_nodes_t generate();
      
      protected:
         location_t empty_location_;
         virtual void timestamp_info_impl() const;
   };
}

#endif //h_4b03135f_1e7a_4294_b464_d7802b6e8628
