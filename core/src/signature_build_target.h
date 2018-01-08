#if !defined(h_5db3364a_84ce_4a8f_a5bc_bde34dcef26f)
#define h_5db3364a_84ce_4a8f_a5bc_bde34dcef26f

#include <hammer/core/generated_build_target.h>

namespace hammer
{

class signature_build_target : public generated_build_target
{
   public:
		signature_build_target(const main_target* mt,
		                       const std::string& n,
		                       const target_type* t,
		                       const feature_set* f)
		   : generated_build_target(mt, n, t, f)
		{}

   protected:
      void timestamp_info_impl() const override;
};

}

#endif //h_5db3364a_84ce_4a8f_a5bc_bde34dcef26f
