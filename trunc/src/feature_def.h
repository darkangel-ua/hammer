#pragma once
#include <vector>
#include <string>
#include "feature_type.h"

namespace hammer
{
   class feature_def
   {
      public:
         feature_def(const std::string& name, 
                     const std::vector<std::string>& legal_values,
                     feature_type fdtype);
         
         const std::string& name() const { return name_; }
         void set_default(const std::string& v);
         const std::string& get_default() const { return default_; }
         ~feature_def(); 

      private:
         std::string name_;
         std::vector<std::string> legal_values_;
         feature_type type_;
         std::string default_;
   };
}