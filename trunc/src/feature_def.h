#pragma once
#include <vector>
#include <string>
#include <map>
#include "feature_attributes.h"

namespace hammer
{
   class feature_set;
   class feature_def
   {
      public:
         feature_def(const std::string& name, 
                     const std::vector<std::string>& legal_values,
                     feature_attributes fdtype);
         
         const std::string& name() const { return name_; }
         void set_default(const std::string& v);
         const std::string& get_default() const { return default_; }
         feature_attributes attributes() const { return attributes_; }
         void compose(const std::string& value, feature_set* c); // take ownership of c
         size_t composite_size(const std::string& value) const;
         ~feature_def(); 
         bool operator ==(const feature_def& rhs) const { return this == &rhs; }

      private:
         struct component_t
         {
            component_t(){}
            component_t(feature_set* c, size_t s) : components_(c), size_(s) {}
            feature_set* components_;
            size_t size_;
         };

         typedef std::map<std::string /*feature value*/, component_t> components_t;

         std::string name_;
         std::vector<std::string> legal_values_;
         feature_attributes attributes_;
         std::string default_;
         components_t components_;
   };
}