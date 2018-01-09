#if !defined(h_f9def316_831c_46d4_ba46_32dc6a216cf1)
#define h_f9def316_831c_46d4_ba46_32dc6a216cf1

#include "typed_meta_target.h"

namespace hammer
{
   class testing_meta_target : public typed_meta_target
   {
      public:
         testing_meta_target(hammer::project* p, 
                             const std::string& name,
                             const requirements_decl& req, 
                             const requirements_decl& usage_req,
                             const target_type& t);
      protected:
         main_target* construct_main_target(const main_target* owner,
                                            const feature_set* properties) const override;
   };
}

#endif //h_f9def316_831c_46d4_ba46_32dc6a216cf1
