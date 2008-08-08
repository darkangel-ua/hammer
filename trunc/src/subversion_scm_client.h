#pragma once

#include "scm_client.h"

namespace hammer
{
   class subversion_scm_client : public scm_client
   {
      public:
         virtual void checkout(const location_t& path, const std::string& uri) const;
   };
}