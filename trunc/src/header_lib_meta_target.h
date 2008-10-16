#if !defined(h_5dfc9743_f59c_458d_9f35_b8d36006bf2e)
#define h_5dfc9743_f59c_458d_9f35_b8d36006bf2e

#include "typed_meta_target.h"

namespace hammer
{
   class header_lib_meta_target : public typed_meta_target
   {
      public:
         header_lib_meta_target(hammer::project* p, const pstring& name, 
                                const requirements_decl& requirements,
                                const requirements_decl& usage_requirements);
      protected:
         virtual void compute_usage_requirements(feature_set& result, 
                                                 const feature_set& full_build_request,
                                                 const feature_set& computed_usage_requirements) const;
   };
}

#endif //h_5dfc9743_f59c_458d_9f35_b8d36006bf2e
