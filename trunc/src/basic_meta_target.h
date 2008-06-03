#pragma once

#include "pstring.h"
#include "requirements_decl.h"

namespace hammer
{
   class feature_set;

   class basic_meta_target
   {
      public:
         basic_meta_target(const pstring& name, 
                           const requirements_decl& req,
                           const requirements_decl& usage_req);
         basic_meta_target() {}
         virtual ~basic_meta_target();
         const pstring& name() const { return name_; }
         void name(const pstring& v) { name_ = v; }
         const requirements_decl& usage_requirements() const { return usage_requirements_; }
         const requirements_decl& requirements() const { return requirements_; }
         void usage_requirements(const requirements_decl& r) { usage_requirements_ = r; }
         void requirements(const requirements_decl& r) { requirements_ = r; }
         virtual const pstring& location() const = 0;

      private:
         pstring name_;
         requirements_decl requirements_;
         requirements_decl usage_requirements_;
   };
}