#if !defined(h_81683f04_032b_4933_879b_73dd9ba4fd87)
#define h_81683f04_032b_4933_879b_73dd9ba4fd87

#include "main_target.h"

namespace hammer
{
   class obj_main_target : public main_target
   {
      public:
         obj_main_target(const hammer::meta_target* mt, 
                         const pstring& name, 
                         const feature_set* props,
                         pool& p);
         virtual std::vector<boost::intrusive_ptr<hammer::build_node> > generate();
   };
}

#endif //h_81683f04_032b_4933_879b_73dd9ba4fd87
