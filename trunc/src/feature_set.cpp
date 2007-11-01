#include "stdafx.h"
#include "feature_set.h"
#include "feature_registry.h"

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
}