#include "stdafx.h"
#include "feature_def.h"

namespace hammer
{
   feature_def::feature_def(const std::string& name, 
                            const std::vector<std::string>& legal_values,
                            feature_type fdtype)
                           :
                            name_(name), legal_values_(legal_values),
                            type_(fdtype)
   {
   }
   
   feature_def::~feature_def()
   {
   }
}
