#pragma once
#include "feature_base.h"
#include "subfeature_def.h"

namespace hammer
{
   class subfeature : public feature_base
   {
      public:
         subfeature(const subfeature_def* def, const pstring& value);
         const subfeature_def& definition() const { return *static_cast<const subfeature_def*>(definition_); }
   };
}