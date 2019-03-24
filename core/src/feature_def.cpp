#include "stdafx.h"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/feature_def.h>

using namespace std;

namespace hammer {

feature_def::feature_def(const std::string& name, 
                         const legal_values_t& legal_values,
                         feature_attributes attributes)
   : name_(name),
     legal_values_(legal_values),
     attributes_(attributes)
{
   map<feature_value_ns_ptr, string> defaults;
   for (auto& lv : legal_values) {
      if (defaults.find(lv.ns_) == defaults.end()) {
         defaults.insert({lv.ns_, lv.value_});
         defaults_.push_back(lv);
      }
   }
}

void feature_def::set_default(const std::string& v)
{
   auto i = find_legal_value(v);
   if (!is_legal_value(v))
      throw std::runtime_error("The value '" + v + "' is not in legal values list for feature '" + name_ + "'.");

   // find default value for v namespace and replace it
   auto di = find_if(defaults_.begin(), defaults_.end(), [&](const legal_value& v){ return v.ns_ == i->ns_; });
   *di = *i;
}

bool feature_def::defaults_contains(const std::string& value) const
{
   return find_if(defaults_.begin(), defaults_.end(), [&](const legal_value& lv){ return lv.value_ == value; }) != defaults_.end();
}

void feature_def::extend_legal_values(const std::string& new_legal_value,
                                      feature_value_ns_ptr ns)
{
   if (is_legal_value(new_legal_value))
      throw std::runtime_error("Legal value '" + new_legal_value + "' already registred for feature '" + name_ + "'");

   legal_values_.push_back({new_legal_value, ns});
   if (legal_values_.size() == 1) {
      defaults_.clear();
      defaults_.push_back(legal_values_.front());
   } else {
      // find default value for v namespace and add
      auto di = find_if(defaults_.begin(), defaults_.end(), [&](const legal_value& v){ return v.ns_ == ns; });
      if (di == defaults_.end())
         defaults_.push_back(legal_values_.back());
   }
}

feature_def::legal_values_t::const_iterator
feature_def::find_legal_value(const string& value) const
{
   return std::find_if(legal_values_.begin(), legal_values_.end(), [&](const legal_value& lv) { return lv.value_ == value; });
}

bool feature_def::is_legal_value(const std::string& v) const
{
   return find_legal_value(v) != legal_values_.end();
}

const feature_value_ns_ptr&
feature_def::get_legal_value_ns(const std::string& value) const
{
   auto i = find_legal_value(value);
   if (i == legal_values_.end())
      throw std::runtime_error("Value '" + value + "' is not legal");

   return i->ns_;
}

void feature_def::compose(const std::string& value,
                          feature_set* c)
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

subfeature_def&
feature_def::add_subfeature(const std::string& subfeature_name)
{
   if (attributes().free || 
       attributes().dependency ||
       attributes().path ||
       attributes().generated)
   {
      throw std::runtime_error("Feature '" + subfeature_name + "' cannot have subfeatures.");
   }

   const auto r = subfeatures_.insert(make_pair(subfeature_name, unique_ptr<subfeature_def>(new subfeature_def(*this, subfeature_name))));
   if (!r.second)
      throw std::runtime_error("Subfeature '" + subfeature_name + "' already defined in feature_def '" + name() + "'.");

   return *r.first->second;
}

const subfeature_def*
feature_def::find_subfeature(const std::string& name) const
{
   subfeatures_t::const_iterator i = subfeatures_.find(name);
   if (i == subfeatures_.end())
      return nullptr;
   else
      return i->second.get();
}

const subfeature_def&
feature_def::get_subfeature(const std::string& subfeature_name) const
{
   const subfeature_def* sd = find_subfeature(subfeature_name);
   if (!sd)
      throw std::runtime_error("Subfeature '" + subfeature_name + "' not founded in feature definition '" + name() + "'");
   else
      return *sd;
}

subfeature_def&
feature_def::get_subfeature(const std::string& name)
{
   return const_cast<subfeature_def&>(const_cast<const feature_def*>(this)->get_subfeature(name));
}

const subfeature_def*
feature_def::find_subfeature_for_value(const std::string& feature_value,
                                       const std::string& value) const
{
   for(subfeatures_t::const_iterator i = subfeatures_.begin(), last = subfeatures_.end(); i != last; ++i)
      if (i->second->is_legal_value(feature_value, value))
         return i->second.get();

   return nullptr;
}

feature_def::~feature_def()
{
}

}
