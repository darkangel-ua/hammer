#pragma once

#include <vector>

namespace hammer
{
   class feature;
   class feature_set
   {
      public:
         typedef std::vector<feature*> features_t;
         void insert(feature* f);

      private:
         features_t features_;
   };
}