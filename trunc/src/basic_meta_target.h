#pragma once

#include "pstring.h"

namespace hammer
{
   class feature_set;

   class basic_meta_target
   {
      public:
         basic_meta_target(const pstring& name, 
                           feature_set* req, 
                           feature_set* usage_req);
         basic_meta_target() {}

         const pstring& name() const { return name_; }
         void name(const pstring& v) { name_ = v; }
         const feature_set& usage_requirements() const { return *usage_requirements_; }
         const feature_set& requirements() const { return *requirements_; }
         virtual const pstring& location() const = 0;

      private:
         pstring name_;
         feature_set* requirements_;
         feature_set* usage_requirements_;
   };
}