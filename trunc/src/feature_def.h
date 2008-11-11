#if !defined(h_87310eb0_8b93_49c6_9804_55a01ca2f7a0)
#define h_87310eb0_8b93_49c6_9804_55a01ca2f7a0

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
                     const std::vector<std::string>& legal_values = std::vector<std::string>(),
                     feature_attributes fdtype = feature_attributes());
         
         const std::string& name() const { return name_; }
         void set_default(const std::string& v);
         const std::string& get_default() const { return default_; }
         feature_attributes attributes() const { return attributes_; }
         feature_attributes& attributes() { return attributes_; }
         void compose(const std::string& value, feature_set* c); // take ownership of c
         void expand_composites(const std::string value, feature_set* fs) const;
         void extend(const std::string& new_legal_value);
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

#endif //h_87310eb0_8b93_49c6_9804_55a01ca2f7a0
