#if !defined(h_82477787_c159_4081_bfa8_ad45d237495c)
#define h_82477787_c159_4081_bfa8_ad45d237495c

#include "file_target.h"

namespace hammer
{
   class generated_target : public file_target
   {
      public:
         generated_target(const main_target* mt, const pstring& n, 
                          const hammer::type* t, const feature_set* f);
         virtual const pstring& location() const;
         virtual std::vector<boost::intrusive_ptr<build_node> > generate();
   };
}

#endif //h_82477787_c159_4081_bfa8_ad45d237495c
