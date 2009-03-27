#if !defined(h_d4ef9a9d_defb_4ea4_bab4_8e1e9c4e31b4)
#define h_d4ef9a9d_defb_4ea4_bab4_8e1e9c4e31b4

#include "basic_target.h"

namespace hammer
{
   class header_lib_target : public basic_target
   {
      public:
         header_lib_target(const main_target* mt, const pstring& name,
                           const target_type* t, const feature_set* f);
         virtual const location_t& location() const { return empty_location_; }
         virtual std::vector<boost::intrusive_ptr<build_node> > generate();
      
      protected:
         location_t empty_location_;
         virtual void timestamp_info_impl() const;
   };
}

#endif //h_d4ef9a9d_defb_4ea4_bab4_8e1e9c4e31b4
