#if !defined(h_42ecd05c_91a6_419a_8239_38d16d85b381)
#define h_42ecd05c_91a6_419a_8239_38d16d85b381

#include <vector>
#include <string>
#include "feature_attributes.h"

namespace hammer
{
   class feature_def_base
   {
      public:
         typedef std::vector<std::string> legal_values_t;
         feature_def_base(const std::string& name, 
                          const legal_values_t& legal_values = legal_values_t(),
                          feature_attributes fdtype = feature_attributes());
         
         const std::string& name() const { return name_; }
         void set_default(const std::string& v);
         const std::string& get_default() const { return default_; }
         feature_attributes attributes() const { return attributes_; }
         feature_attributes& attributes() { return attributes_; }
         void extend_legal_values(const std::string& new_legal_value);
         const legal_values_t& legal_values() const { return legal_values_; }
         bool is_legal_value(const std::string& v) const;

         ~feature_def_base(); 
//         bool operator ==(const feature_def& rhs) const { return this == &rhs; }

      protected:
         std::string name_;
         legal_values_t legal_values_;
         feature_attributes attributes_;
         std::string default_;
   };
}

#endif //h_42ecd05c_91a6_419a_8239_38d16d85b381
