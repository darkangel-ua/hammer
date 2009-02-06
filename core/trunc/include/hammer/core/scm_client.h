#if !defined(h_df3294ec_17ab_4253_9f63_e0cd5eaf4a41)
#define h_df3294ec_17ab_4253_9f63_e0cd5eaf4a41

#include "location.h"

namespace hammer
{
   class scm_client
   {
      public:
         virtual void checkout(const location_t& path, const std::string& uri, bool recursive = true) const = 0;
         virtual void up(const location_t& where_up, const std::string& what_up, bool recursive = true) const = 0;
         virtual ~scm_client() {};
   };
}

#endif //h_df3294ec_17ab_4253_9f63_e0cd5eaf4a41
