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
         feature_def& get_def(const std::string& name);
         feature_set* make_set();
         feature* create_feature(const char* name, const char* value) const;
         feature* create_feature(const std::string& name, const std::string& value) const
         {
            return create_feature(name.c_str(), value.c_str());
         }

         void add_defaults(feature_set* s) const;
         const feature_def* find_def(const char* feature_name) const;
         ~feature_registry();

      private:
         mutable impl_t* impl_;
   };
}