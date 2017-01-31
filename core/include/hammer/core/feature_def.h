#if !defined(h_87310eb0_8b93_49c6_9804_55a01ca2f7a0)
#define h_87310eb0_8b93_49c6_9804_55a01ca2f7a0

#include <map>
#include <memory>
#include <vector>
#include "feature_attributes.h"
#include "subfeature_def.h"

namespace hammer
{
   class feature_set;
   class feature_registry;

   class feature_def
   {
         // needs to use constructor
         friend class feature_registry;

      public:
         typedef std::vector<std::string> legal_values_t;

         feature_def(const feature_def&) = delete;
         feature_def(feature_def&&) = delete;
         feature_def& operator = (const feature_def&) = delete;

         const std::string& name() const { return name_; }
         void set_default(const std::string& v);
         const std::string& get_default() const { return default_; }
         feature_attributes attributes() const { return attributes_; }
         feature_attributes& attributes() { return attributes_; }
         void extend_legal_values(const std::string& new_legal_value);
         const legal_values_t& legal_values() const { return legal_values_; }
         bool is_legal_value(const std::string& v) const;

         void compose(const std::string& value, feature_set* c); // FIXME: take ownership of c
         void expand_composites(const std::string value, feature_set* fs) const;
         subfeature_def& add_subfeature(const std::string& name);
         const subfeature_def* find_subfeature(const std::string& name) const;
         const subfeature_def& get_subfeature(const std::string& name) const;
         subfeature_def& get_subfeature(const std::string& name);

         const subfeature_def*
         find_subfeature_for_value(const std::string& feature_value,
                                   const std::string& value) const;

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
         typedef std::map<std::string /*subfeature name*/, std::unique_ptr<subfeature_def> > subfeatures_t;

         // only used by feature_registry
         feature_def(const std::string& name,
                     const legal_values_t& legal_values,
                     feature_attributes attributes);

         std::string name_;
         legal_values_t legal_values_;
         feature_attributes attributes_;
         std::string default_;

         components_t components_;
         subfeatures_t subfeatures_;
   };
}

#endif //h_87310eb0_8b93_49c6_9804_55a01ca2f7a0
