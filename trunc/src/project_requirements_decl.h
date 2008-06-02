#pragma once

#include <string>
#include "requirements_decl.h"

namespace hammer
{
   class project_requirements_decl
   {
      public:
         project_requirements_decl(const std::string& name, 
                                   const requirements_decl& r);
         const std::string& name() const { return name_; }
         const requirements_decl& requirements() const { return requirements_; }

      private:
         std::string name_;
         requirements_decl requirements_;
   };
}