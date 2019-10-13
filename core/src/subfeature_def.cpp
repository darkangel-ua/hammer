#include "stdafx.h"
#include <stdexcept>
#include <algorithm>
#include <hammer/core/subfeature_def.h>
#include <hammer/core/feature_def.h>

namespace hammer{

subfeature_def::subfeature_def(const feature_def& owner,
                               const std::string& name)
   : owner_(&owner),
     name_(name)
{
}

static const subfeature_def::legal_values_t empty_legal_values;

const subfeature_def::legal_values_t&
subfeature_def::legal_values(const std::string& feature_value) const
{
   if (!owner_->is_legal_value(feature_value))
      throw std::runtime_error("Value '" + feature_value +"' is not legal for feature '" + owner_->name() + "'");

   auto i = all_legal_values_.find(feature_value);
   if (i == all_legal_values_.end())
      return empty_legal_values;
   else
      return i->second;
}

bool subfeature_def::is_legal_value(const std::string& feature_value,
                                    const std::string& value) const
{
   auto i = all_legal_values_.find(feature_value);
   if (i == all_legal_values_.end())
      return false;

   return find(i->second.begin(), i->second.end(), value) != i->second.end();
}

void subfeature_def::extend_legal_values(const std::string& feature_value,
                                         const std::string& new_legal_value)
{
   if (!owner_->is_legal_value(feature_value))
      throw std::runtime_error("Value '" + feature_value +"' is not legal for feature '" + owner_->name() + "'");

   auto i = all_legal_values_.find(feature_value);
   if (i == all_legal_values_.end()) {
      all_legal_values_.insert({feature_value, {new_legal_value}});
      return;
   }

   auto ii = find(i->second.begin(), i->second.end(), new_legal_value);
   if (ii != i->second.end())
      throw std::runtime_error("Feature '" + owner_->name() + "' subfeature '" + name_ + "' already has '" + new_legal_value + "' as legal value");

   i->second.push_back(new_legal_value);
}

}
