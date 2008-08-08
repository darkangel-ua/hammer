#pragma once
#include "location.h"

namespace hammer
{
   class scm_client
   {
      public:
         virtual void checkout(const location_t& path, const std::string& uri) const = 0;
         virtual ~scm_client() {};
   };
}