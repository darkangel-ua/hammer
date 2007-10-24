#include "stdafx.h"
#include "feature_set.h"

namespace hammer{
   void feature_set::insert(feature* f)
   {
      features_.push_back(f);
   }
}