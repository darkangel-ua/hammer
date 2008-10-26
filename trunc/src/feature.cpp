#include "stdafx.h"
#include "feature.h"

using namespace std;

namespace hammer{

feature::feature(const feature_def* def, const pstring& value)
                : 
                 def_(def), value_(value)
{
}

bool feature::operator == (const feature& rhs) const
{
   if (name() != rhs.name())
      return false;

   if (attributes().dependency)
      return get_dependency_data().source_ == rhs.get_dependency_data().source_ &&
             get_path_data().target_ == rhs.get_path_data().target_;
   if (attributes().path)
      return value() == rhs.value() &&
             get_path_data().target_ == rhs.get_path_data().target_;
   if (attributes().generated)
      return value() == rhs.value() &&
             get_generated_data().target_ == rhs.get_generated_data().target_;

   return value() == rhs.value();
}

}