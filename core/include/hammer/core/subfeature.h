#if !defined(h_85f86c30_04b2_4778_838d_12e9b607ed32)
#define h_85f86c30_04b2_4778_838d_12e9b607ed32

#include "feature_base.h"
#include "subfeature_def.h"

namespace hammer
{
   class subfeature : public feature_base
   {
      public:
         subfeature(const subfeature_def* def, const pstring& value);
         const subfeature_def& definition() const { return *static_cast<const subfeature_def*>(definition_); }
   };
}

#endif //h_85f86c30_04b2_4778_838d_12e9b607ed32
