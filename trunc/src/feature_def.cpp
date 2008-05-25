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

static size_t compute_size(const feature_set& fs)
{
   size_t result = 0;
   for(feature_set::const_iterator i = fs.begin(), last = fs.end(); i != last; ++i)
   {
      ++result;
      if ((**i).attributes().composite)
         result += (**i).def().composite_size((**i).value().to_string());
   }

   return result + 1;
}

void feature_def::compose(const std::string& value, feature_set* c)
{
   assert(attributes().composite);

   components_t::const_iterator i = components_.find(value);
   if (i != components_.end())
      throw std::runtime_error("Feature components already defined.");

   components_.insert(make_pair(value, component_t(c, compute_size(*c))));
}

size_t feature_def::composite_size(const std::string& value) const
{
   assert(attributes().composite);
   components_t::const_iterator i = components_.find(value);
   if (i == components_.end())
      throw std::runtime_error("There is no such value '" + value + "' in legals values.");
   
   return i->second.size_;  
}

feature_def::~feature_def()
{
}

}
