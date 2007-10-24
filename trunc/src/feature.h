#pragma once
#include <string>

namespace hammer
{
   struct feature_type
   {
      typedef enum {path, free, dependency} value;
   };

   class feature
   {
      public:
         feature(const std::string& name, const std::string& value, feature_type::value t);
      
      private:
         std::string name_;
         std::string value_;
         feature_type::value type_;
   };
}