#pragma once

#include "feature_def.h"

namespace hammer
{
   class feature_registry
   {
      public:
         void add_def(std::auto_ptr<feature_def> def);
         feature_registry();
         ~feature_registry();

      private:
         typedef boost::ptr_map<const std::string, feature_def> defs_t;

         defs_t defs_;
   };
}