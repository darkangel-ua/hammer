#if !defined(h_8c4e66a7_204c_4ef1_b9f9_4decf9327607)
#define h_8c4e66a7_204c_4ef1_b9f9_4decf9327607

#include <hammer/core/alias_meta_target.h>

namespace hammer
{
   class version_aliase_meta_target : public alias_meta_target
   {
      public:
         version_aliase_meta_target(hammer::project* p, 
                                    const pstring& name,
                                    const pstring& version,
                                    const sources_decl* sources);
   };
}

#endif //h_8c4e66a7_204c_4ef1_b9f9_4decf9327607
