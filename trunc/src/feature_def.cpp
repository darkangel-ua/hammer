#include "stdafx.h"
#include "feature_def.h"
#include <algorithm>

namespace hammer
{

feature_def::feature_def(const std::string& name, 
                         const std::vector<std::string>& legal_values,
                         feature_attributes fdtype)
                        :
                         name_(name), legal_values_(legal_values),
                         type_(fdtype)
{
   if (!legal_values_.empty())
      default_ = legal_values_[0];
}

void feature_def::set_default(const std::string& v)
{
   if (legal_values_.end() != std::find(legal_values_.begin(), legal_values_.end(), v))
   {
      throw std::runtime_error("The value '" + v + "' is not in legal values list for feature '" + name_ + "'.");
   }

   default_ = v;
}

feature_def::~feature_def()
{
}

}
