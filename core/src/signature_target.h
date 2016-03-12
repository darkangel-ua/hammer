#if !defined(h_5db3364a_84ce_4a8f_a5bc_bde34dcef26f)
#define h_5db3364a_84ce_4a8f_a5bc_bde34dcef26f

#include <hammer/core/generated_target.h>

namespace hammer
{

class signature_target : public generated_target
{
   public:
      signature_target(const main_target* mt, const pstring& n, 
                       const target_type* t, const feature_set* f)
         : generated_target(mt, n, t, f)
      {}

   protected:
      virtual void timestamp_info_impl() const;
};

}


#endif //h_5db3364a_84ce_4a8f_a5bc_bde34dcef26f
