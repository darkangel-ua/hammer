#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <hammer/core/feature.h>
#include <hammer/core/subfeature.h>

using namespace std;

namespace hammer {

static
feature_value_ns_ptr global_ns;

const feature_value_ns_ptr&
feature::get_value_ns() const {
   if (attributes().no_checks)
      return global_ns;
   else
      return definition_->get_legal_value_ns(value());
}

feature::feature(const feature_def* def,
                 std::string value,
                 const project& p)
   : definition_(def),
     value_(std::move(value)),
     path_data_{&p}
{
}

feature::feature(const feature_def* def,
                 source_decl s)
   : definition_(def),
     dependency_data_{std::move(s)}
{
}

feature::feature(const feature_def* def,
                 std::string value,
                 const basic_target& generated_target)
   : definition_(def),
     value_(std::move(value)),
     generated_data_{&generated_target}
{
}

feature::feature(const feature_def* def,
                 std::string value,
                 subfeatures_t subfeatures)
   : definition_(def),
     value_(std::move(value)),
     subfeatures_(std::move(subfeatures))
{
   assert(def && "Definition cannot be NULL");

   if (!attributes().free &&
       !attributes().no_checks && 
       !def->is_legal_value(value_))
   {
      throw std::runtime_error("Value '" + value_ + "' is not legal for feature '" + name() + "'");
   }
}

bool feature::equal_without_subfeatures(const feature& rhs) const {
   if (name() != rhs.name())
      return false;

   // if target_path is global target doesn't matter
   if (attributes().dependency)
      return get_dependency_data().source_ == rhs.get_dependency_data().source_ &&
             (get_dependency_data().source_.target_path_is_global() ||
              get_path_data().project_ == rhs.get_path_data().project_);

   if (attributes().path)
      return value() == rhs.value() &&
             get_path_data().project_ == rhs.get_path_data().project_;
   if (attributes().generated)
      return value() == rhs.value() &&
             get_generated_data().target_ == rhs.get_generated_data().target_;

   if (value() != rhs.value())
      return false;

   return true;
}

bool feature::operator == (const feature& rhs) const {
   if (!equal_without_subfeatures(rhs))
      return false;

   if (subfeatures().size() != rhs.subfeatures().size())
      return false;

   // std::equal
   auto s_first = subfeatures().begin();
   auto s_last = subfeatures().end();
   auto rhs_i = rhs.subfeatures().begin();
   for (; s_first != s_last; ++s_first, ++rhs_i)
      if (**s_first != **rhs_i)
         return false;

   return true;
}

bool feature::contains(const feature& f) const {
   if (!equal_without_subfeatures(f))
      return false;

   for (const subfeature* sf : f.subfeatures()) {
      if (!find_subfeature(*sf))
         return false;
   }

   return true;
}

bool feature::operator < (const feature& rhs) const {
   if (this == &rhs)
      return false;

   if (definition_ != rhs.definition_)
      return definition_ < rhs.definition_;

   if (this->value() != rhs.value())
      return this->value() < rhs.value();

   if (attributes().dependency)
      if (get_dependency_data() != rhs.get_dependency_data())
         return get_dependency_data() < rhs.get_dependency_data();

   if (attributes().path)
      if (get_path_data() != rhs.get_path_data())
         return get_path_data() < rhs.get_path_data();

   if (attributes().generated)
      if (get_generated_data() != rhs.get_generated_data())
         return get_generated_data() < rhs.get_generated_data();

   if (subfeatures_.size() != rhs.subfeatures_.size())
      return subfeatures_.size() < rhs.subfeatures_.size();

   for (auto i = subfeatures_.begin(), last = subfeatures_.end(), outer = rhs.subfeatures_.begin(); i != last; ++i, ++outer)
      if (*i != *outer)
         return *i < *outer;

   return false;
}

const subfeature*
feature::find_subfeature(const subfeature& v) const {
   auto i = std::find(subfeatures_.begin(), subfeatures_.end(), &v);
   return i == subfeatures_.end() ? nullptr : *i;
}

const subfeature*
feature::find_subfeature(const std::string& v) const {
   for (auto& i : subfeatures_)
      if (i->name() == v)
         return i;
   
   return nullptr;
}

}
