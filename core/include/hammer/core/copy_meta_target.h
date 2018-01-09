#if !defined(h_c27ea7bd_f269_4661_90de_ab303948b91a)
#define h_c27ea7bd_f269_4661_90de_ab303948b91a

#include "meta_target.h"

namespace hammer
{
   class copy_meta_target : public meta_target
   {
      public:
         copy_meta_target(hammer::project* p, const std::string& name,
                          const requirements_decl& props,
                          const requirements_decl& usage_req);
      protected:
         main_target* construct_main_target(const main_target* owner,
			                                   const feature_set* properties) const override;
   };
}

#endif //h_c27ea7bd_f269_4661_90de_ab303948b91a
