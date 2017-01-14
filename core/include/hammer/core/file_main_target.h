#if !defined(h_dc46d308_0d91_45de_9656_544e2f8e7007)
#define h_dc46d308_0d91_45de_9656_544e2f8e7007

#include <hammer/core/main_target.h>

namespace hammer{

class file_main_target : public main_target
{
   public:
      file_main_target(const basic_meta_target* mt,
                       const pstring& name, 
                       const pstring& filename,
                       const feature_set* props,
                       const target_type& t,
                       pool& p);
         virtual std::vector<boost::intrusive_ptr<hammer::build_node> > generate();
      
      private:
         const target_type* type_;
         pstring filename_;
};

}

#endif //h_dc46d308_0d91_45de_9656_544e2f8e7007
