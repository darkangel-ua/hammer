#if !defined(h_87310eb0_8b93_49c6_9804_55a01ca2f7a0)
#define h_87310eb0_8b93_49c6_9804_55a01ca2f7a0

#include <map>
#include "feature_def_base.h"
#include "subfeature_def.h"

namespace hammer
{
   class feature_set;
   // FIXME: inheritance here I think this is a bad idea
   class feature_def : public feature_def_base
   {
      public:
         explicit feature_def(const std::string& name, 
                              const legal_values_t& legal_values = legal_values_t(),
                              feature_attributes fdtype = feature_attributes());
         
         void compose(const std::string& value, feature_set* c); // FIXME: take ownership of c
         void expand_composites(const std::string value, feature_set* fs) const;
         void add_subfeature(const subfeature_def& s);
         const subfeature_def* find_subfeature(const std::string& name) const;
         const subfeature_def& get_subfeature(const std::string& name) const;
         subfeature_def& get_subfeature(const std::string& name);
         const subfeature_def* find_subfeature_for_value(const std::string& value) const;

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
         typedef std::map<std::string /*subfeature name*/, subfeature_def> subfeatures_t;

         components_t components_;
         subfeatures_t subfeatures_;
   };
}

#endif //h_87310eb0_8b93_49c6_9804_55a01ca2f7a0
