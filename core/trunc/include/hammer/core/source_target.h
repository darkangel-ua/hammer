#if !defined(h_000faec0_e1c4_47aa_911a_31a1d9761f31)
#define h_000faec0_e1c4_47aa_911a_31a1d9761f31

#include "pstring.h"
#include "file_target.h"

namespace hammer
{
   class main_target;
   class target_type;
   class engine;
   class feature_set;

   class source_target : public file_target
   {
      public:  
         source_target(const main_target* mt, const pstring& name, 
                       const target_type* t, const feature_set* f);
         virtual std::vector<boost::intrusive_ptr<build_node> > generate();
   };
}

#endif //h_000faec0_e1c4_47aa_911a_31a1d9761f31
