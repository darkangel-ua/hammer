#pragma once

#include "feature_def.h"
#include <map>

namespace hammer
{
   class feature_set;
   class feature;
   class pool;
   class feature_registry
   {
      public:
         struct impl_t;
         feature_registry(pool* p);
         void add_def(const feature_def& def);
         feature_set* make_set();
         feature* create_feature(const char* name, const char* value);

         ~feature_registry();

      private:
         impl_t* impl_;
   };
}