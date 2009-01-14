#if !defined(h_f9def316_831c_46d4_ba46_32dc6a216cf1)
#define h_f9def316_831c_46d4_ba46_32dc6a216cf1

#include "typed_meta_target.h"

namespace hammer
{
   class testing_meta_target : public typed_meta_target
   {
      public:
         testing_meta_target(hammer::project* p, 
                             const pstring& name, 
                             const requirements_decl& req, 
                             const requirements_decl& usage_req,
                             const hammer::type& t);
      protected:
         virtual sources_decl compute_additional_sources(const main_target& owner) const;
         virtual bool is_cachable(const main_target* owner) const { return false; }
   };
}

#endif //h_f9def316_831c_46d4_ba46_32dc6a216cf1
