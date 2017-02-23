#if !defined(h_198b06a7_e428_4e5f_9a34_02dad6b15492)
#define h_198b06a7_e428_4e5f_9a34_02dad6b15492

#include <hammer/core/main_target.h>

namespace hammer{

class testing_main_target : public main_target
{
   public:
      testing_main_target(const basic_meta_target* mt,
                          const std::string& name,
                          const target_type* t, 
                          const feature_set* props);
   protected:
      virtual location_t intermediate_dir_impl() const;
};

}

#endif //h_198b06a7_e428_4e5f_9a34_02dad6b15492
