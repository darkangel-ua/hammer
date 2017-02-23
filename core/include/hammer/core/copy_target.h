#if !defined(h_9cb0baed_600a_4d7b_b97e_625b61be5f46)
#define h_9cb0baed_600a_4d7b_b97e_625b61be5f46

#include "file_target.h"

namespace hammer
{
   class copy_target : public file_target
   {
      public:
         copy_target(const main_target* mt, const std::string& name,
                     const target_type* t, const feature_set* f);
         virtual const location_t& location() const;
   };
}

#endif //h_9cb0baed_600a_4d7b_b97e_625b61be5f46
