#if !defined(h_43216752_a5e6_4909_93f4_82f3fd795811)
#define h_43216752_a5e6_4909_93f4_82f3fd795811

#include "meta_target.h"

namespace hammer
{
   class lib_meta_target : public meta_target
   {
      public:
         lib_meta_target(hammer::project* p, const pstring& name, 
                         const requirements_decl& req,
                         const requirements_decl& usage_req);
      
      protected:
         virtual main_target* construct_main_target(const feature_set* properties) const;
   };
}

#endif //h_43216752_a5e6_4909_93f4_82f3fd795811
