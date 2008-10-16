#if !defined(h_4280f83e_47fb_4cac_b083_0d1583288878)
#define h_4280f83e_47fb_4cac_b083_0d1583288878

#include "meta_target.h"

namespace hammer
{
   class obj_meta_target : public meta_target
   {
      public:
         obj_meta_target(hammer::project* p, const pstring& name, 
                         const requirements_decl& req, 
                         const requirements_decl& usage_req);
      protected:
         virtual main_target* construct_main_target(const feature_set* properties) const;
   };
}

#endif //h_4280f83e_47fb_4cac_b083_0d1583288878
