#include "stdafx.h"
#include "feature_def.h"
#include <algorithm>
#include <cassert>
#include "feature_set.h"
#include "feature.h"

namespace hammer
{

feature_def::feature_def(const std::string& name, 
                         const std::vector<std::string>& legal_values,
                         feature_attributes attrs)
                        :
                         name_(name), legal_values_(legal_values),
                         attributes_(attrs)
{
   if (!legal_values_.empty())
      default_ = legal_values_[0];
}

void feature_def::set_default(const std::string& v)
{
   if (legal_values_.end() != std::find(legal_values_.begin(), legal_values_.end(), v))
      throw std::runtime_error("The value '" + v + "' is not in legal values list for feature '" + name_ + "'.");

   default_ = v;
}

void feature_def::compose(const std::string& value, feature_set* c)
{
   assert(attributes().composite);

   components_t::const_iterator i = components_.find(value);
   if (i != components_.end())
      throw std::runtime_error("Feature components already defined.");

   components_.insert(make_pair(value, component_t(c, 0)));
}

void feature_def::expand_composites(const std::string value, feature_set* fs) const
{
   assert(attributes().composite);
   components_t::const_iterator i = components_.find(value);
   if (i == components_.end())
      throw std::runtime_error("The feature def '" + name() + "' doesn't have composite value '" + value + "'.");

   for(feature_set::const_iterator f = i->second.components_->begin(), last = i->second.components_->end(); f != last; ++f)
      fs->join(*f);
}

void feature_def::extend(const std::string& new_legal_value)
{
   if (find(legal_values_.begin(), legal_values_.end(), new_legal_value) != legal_values_.end())
      throw std::runtime_error("Legal value '" + new_legal_value + "' already registred for feature '" + name_ + "'");

   legal_values_.push_back(new_legal_value);
   if (legal_values_.size() == 1)
      default_ = legal_values_[0];
}

feature_def::~feature_def()
{
}

}
