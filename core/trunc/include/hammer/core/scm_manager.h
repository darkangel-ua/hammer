#if !defined(h_10c35936_31ea_44e7_930d_8e93ae4e66b8)
#define h_10c35936_31ea_44e7_930d_8e93ae4e66b8

#include <string>
#include "scm_client.h"
#include <boost/ptr_container/ptr_map.hpp>

namespace hammer
{
   class scm_manager
   {
      public:
         scm_manager();
         const scm_client* find(const std::string& id) const;
      
      private:
         typedef boost::ptr_map<std::string /* scm id*/, scm_client /* implementation*/> scm_clients;

         scm_clients scm_clients_;
   };
}

#endif //h_10c35936_31ea_44e7_930d_8e93ae4e66b8
