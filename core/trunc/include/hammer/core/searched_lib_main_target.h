#if !defined(h_0a9ee961_c72c_4beb_a22a_b562db749dfd)
#define h_0a9ee961_c72c_4beb_a22a_b562db749dfd

#include "main_target.h"

namespace hammer
{
   class searched_lib_main_target : public main_target
   {
      public:
         searched_lib_main_target(const hammer::meta_target* mt, 
                                  const pstring& name, 
                                  const feature_set* props,
                                  pool& p);
         virtual std::vector<boost::intrusive_ptr<hammer::build_node> > generate();

   };
}

#endif //h_0a9ee961_c72c_4beb_a22a_b562db749dfd
