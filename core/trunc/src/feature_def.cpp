#include "stdafx.h"
#include "feature_def.h"
#include <algorithm>
#include <cassert>
#include "feature_set.h"
#include "feature.h"

using namespace std;

namespace hammer
{

feature_def::feature_def(const std::string& name, 
                         const legal_values_t& legal_values,
                         feature_attributes attrs)
                        :
                         feature_def_base(name, legal_values, attrs)
{
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

void feature_def::add_subfeature(const subfeature_def& s)
{
   if (attributes().free || 
       attributes().dependency ||
       attributes().path ||
       attributes().generated)
   {
      throw std::runtime_error("Feature '" + name() + "' cannot have subfeatures.");
   }

   if (!subfeatures_.insert(make_pair(s.name(), s)).second)
      throw std::runtime_error("Subfeature '" + s.name() + "' already defined in feature_def '" + name() + "'.");
}

const subfeature_def* feature_def::find_subfeature(const std::string& name) const
{
   subfeatures_t::const_iterator i = subfeatures_.find(name);
   if (i == subfeatures_.end())
      return NULL;
   else
      return &i->second;
}

const subfeature_def& feature_def::get_subfeature(const std::string& subfeature_name) const
{
   const subfeature_def* sd = find_subfeature(subfeature_name);
   if (sd == NULL)
      throw std::runtime_error("Subfeature '" + subfeature_name + "' not founded in feature definition '" + name() + "'");
   else
      return *sd;
}

subfeature_def& feature_def::get_subfeature(const std::string& name)
{
   return const_cast<subfeature_def&>(const_cast<const feature_def*>(this)->get_subfeature(name));
}

const subfeature_def* feature_def::find_subfeature_for_value(const std::string& value) const
{
   for(subfeatures_t::const_iterator i = subfeatures_.begin(), last = subfeatures_.end(); i != last; ++i)
      if (i->second.is_legal_value(value))
         return &i->second;

   return NULL;
}

feature_def::~feature_def()
{
}

}
