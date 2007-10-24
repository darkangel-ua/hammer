#pragma once
#include <vector>
#include <string>

namespace hammer
{
   class feature_def
   {
      public:
         feature_def(const std::string& name, const std::vector<std::string>& legal_values);
      
      private:
         std::string name_;
         std::vector<std::string> legal_values_;
   };
}