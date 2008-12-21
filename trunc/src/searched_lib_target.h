#if !defined(h_6321a33a_7df3_44b9_8861_dc2ce5a7690c)
#define h_6321a33a_7df3_44b9_8861_dc2ce5a7690c

#include "file_target.h"

namespace hammer
{
   class searched_lib_target : public file_target
   {
      public:
         searched_lib_target(const main_target* mt, const pstring& name,
                             const hammer::type* t, const feature_set* f) : file_target(mt, name, t, f) 
         {
         }
         
         virtual const location_t& location() const { return empty_location_; }

      protected:
         location_t empty_location_;
         virtual void timestamp_info_impl() const;
   };
}

#endif //h_6321a33a_7df3_44b9_8861_dc2ce5a7690c
