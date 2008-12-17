#if !defined(h_ec3dfa36_40f1_45e9_a457_788faf7eaf27)
#define h_ec3dfa36_40f1_45e9_a457_788faf7eaf27

#include "basic_target.h"

namespace hammer
{
   class file_target : public basic_target
   {
      public:
         file_target(const main_target* mt, const pstring& name,
                     const hammer::type* t, const feature_set* f) : basic_target(mt, name, t, f) 
         {
         }

         virtual std::vector<boost::intrusive_ptr<build_node> > generate();
      
      protected:
         virtual void timestamp_info_impl() const;

   };
}

#endif //h_ec3dfa36_40f1_45e9_a457_788faf7eaf27
