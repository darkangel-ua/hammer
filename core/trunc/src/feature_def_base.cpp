#include "stdafx.h"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/feature_def_base.h>

namespace hammer
{

feature_def_base::feature_def_base(const std::string& name, 
                                   const std::vector<std::string>& legal_values,
                                   feature_attributes attrs)
                                  :
                                   name_(name), legal_values_(legal_values),
                                   attributes_(attrs)
{
   if (!legal_values_.empty())
      default_ = legal_values_[0];
}

void feature_def_base::set_default(const std::string& v)
{
   if (legal_values_.end() == std::find(legal_values_.begin(), legal_values_.end(), v))
      throw std::runtime_error("The value '" + v + "' is not in legal values list for feature '" + name_ + "'.");

   default_ = v;
}

void feature_def_base::extend_legal_values(const std::string& new_legal_value)
{
   if (find(legal_values_.begin(), legal_values_.end(), new_legal_value) != legal_values_.end())
      throw std::runtime_error("Legal value '" + new_legal_value + "' already registred for feature '" + name_ + "'");

   legal_values_.push_back(new_legal_value);
   if (legal_values_.size() == 1)
      default_ = legal_values_[0];
}

feature_def_base::~feature_def_base()
{
}

bool feature_def_base::is_legal_value(const std::string& v) const
{
   return std::find(legal_values_.begin(), legal_values_.end(), v) != legal_values_.end();
}

}
