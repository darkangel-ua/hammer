#if !defined(h_911b15ef_ec54_4217_bee8_b61c7584f9f5)
#define h_911b15ef_ec54_4217_bee8_b61c7584f9f5

#include <string>
#include <vector>
#include <memory>
#include "type_tag.h"

namespace hammer
{
   class type_registry;
   class feature_set;

   class target_type
   {
      public:
         struct suffix_def
         {
            suffix_def(const std::string& suffix) : suffix_(suffix), condition_(NULL) {}
            suffix_def(const char* suffix) : suffix_(suffix), condition_(NULL) {}
            suffix_def(const char* suffix,
                       const feature_set& condition)
               : suffix_(suffix),
                 condition_(&condition)
            {}

            bool operator == (const suffix_def& rhs) const;

            std::string suffix_;
            const feature_set* condition_;
         };

         typedef std::vector<suffix_def> suffixes_t;

         target_type(const type_tag& tag, const suffix_def& suffix);
         target_type(const type_tag& tag, const suffix_def& suffix, const target_type& base);
         target_type(const type_tag& tag, const suffixes_t& suffixes);
         target_type(const type_tag& tag, const suffixes_t& suffixes, const target_type& base);
         
         const type_tag& tag() const { return tag_; }
         const suffixes_t& suffixes() const { return suffixes_; }
         const std::string& suffix_for(const std::string& s, const feature_set& environment) const;
         const std::string& suffix_for(const feature_set& environment) const;
         const target_type* base() const { return base_ ; }

         bool equal_or_derived_from(const target_type& rhs) const;
         std::auto_ptr<target_type> clone(const type_registry& tr) const;

      private:
         type_tag tag_;
         suffixes_t suffixes_;
         const type_registry* owner_;
         const target_type* base_;

         bool equal(const target_type& rhs) const;
   };
}

#endif //h_911b15ef_ec54_4217_bee8_b61c7584f9f5
