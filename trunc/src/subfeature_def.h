#pragma once

#include "feature_def_base.h"

namespace hammer
{
   // FIXME: inheritance here I think is a bad idea
   class subfeature_def : public feature_def_base
   {
      public:
         explicit subfeature_def(const std::string& name, 
                                 const std::vector<std::string>& legal_values = std::vector<std::string>(),
                                 feature_attributes fdtype = feature_attributes());
   };
}