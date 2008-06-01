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
                           feature_set* usage_req);
         basic_meta_target() {}
         virtual ~basic_meta_target();
         const pstring& name() const { return name_; }
         void name(const pstring& v) { name_ = v; }
         const feature_set& usage_requirements() const { return *usage_requirements_; }
         const requirements_decl& requirements() const { return requirements_; }
         virtual const pstring& location() const = 0;

      private:
         pstring name_;
         requirements_decl requirements_;
         feature_set* usage_requirements_;
   };
}