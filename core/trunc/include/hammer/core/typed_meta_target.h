#if !defined(h_0ff2ca9d_f8b6_492a_ba59_edacea125380)
#define h_0ff2ca9d_f8b6_492a_ba59_edacea125380

#include "meta_target.h"

namespace hammer
{
   class target_type;

   class typed_meta_target : public meta_target
   {
      public:
         typed_meta_target(hammer::project* p, 
                           const pstring& name, 
                           const requirements_decl& req, 
                           const requirements_decl& usage_req,
                           const target_type& t) :  
            meta_target(p, name, req, usage_req), type_(&t)
         {
         }
         
         const target_type& type() const { return *type_; }

      protected:
         virtual main_target* construct_main_target(const main_target* owner, const feature_set* properties) const;

     private:
        const target_type* type_;
   };
}

#endif //h_0ff2ca9d_f8b6_492a_ba59_edacea125380
