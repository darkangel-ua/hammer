#if !defined(h_295945d1_c56e_43fa_ad45_659b96d6bd21)
#define h_295945d1_c56e_43fa_ad45_659b96d6bd21

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

#endif //h_295945d1_c56e_43fa_ad45_659b96d6bd21
