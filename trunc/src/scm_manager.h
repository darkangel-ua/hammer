#pragma once 
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