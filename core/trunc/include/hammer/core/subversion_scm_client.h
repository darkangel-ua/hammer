#if !defined(h_1f93485c_0d49_4e95_bc15_b005fe658fb0)
#define h_1f93485c_0d49_4e95_bc15_b005fe658fb0

#include "scm_client.h"

namespace hammer
{
   class subversion_scm_client : public scm_client
   {
      public:
         virtual void checkout(const location_t& path, const std::string& uri, bool recursive = true) const;
         virtual void up(const location_t& where_up, const std::string& what_up, bool recursive = true) const;
   };
}

#endif //h_1f93485c_0d49_4e95_bc15_b005fe658fb0
