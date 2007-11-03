#include "stdafx.h"
#include "feature_set.h"
#include "feature_registry.h"
#include "feature.h"

using namespace std;

namespace hammer{

   feature_set::feature_set(feature_registry* fr) : fr_(fr)
   {
   }

   void feature_set::insert(const char* name, const char* value)
   {
      features_.push_back(fr_->create_feature(name, value));
   }

   void feature_set::insert(feature* f)
   {
      features_.push_back(f);
   }

   const feature* feature_set::get(const char* name_) const
   {
      string name(name_);
      for(features_t::const_iterator i = features_.begin(), last = features_.end(); i != last; ++i)
         if ((**i).def()->name() == name)
            return *i;

      throw runtime_error("feature '" + name + "not founded");
   }
}