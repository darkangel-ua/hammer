#if !defined(h_0a9ee961_c72c_4beb_a22a_b562db749dfd)
#define h_0a9ee961_c72c_4beb_a22a_b562db749dfd

#include "main_target.h"

namespace hammer
{
   class searched_lib_main_target : public main_target
   {
      public:
         searched_lib_main_target(const basic_meta_target* mt,
                                  const std::string& name,
                                  const std::string& lib_name,
                                  const feature_set* props,
                                  const target_type& t);
         virtual std::vector<boost::intrusive_ptr<hammer::build_node> > generate() const;
      
      private:
         const target_type* type_;
         const std::string lib_name_;
   };
}

#endif //h_0a9ee961_c72c_4beb_a22a_b562db749dfd
