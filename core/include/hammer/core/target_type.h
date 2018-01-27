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
	class feature;

   class target_type
   {
      public:
         struct suffix_def
         {
            suffix_def() : condition_(NULL) {}
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

         typedef suffix_def prefix_def;
         typedef std::vector<suffix_def> suffixes_t;
         typedef std::vector<prefix_def> prefixes_t;

         target_type(const type_tag& tag, const suffix_def& suffix, const prefix_def& prefix = prefix_def());
         target_type(const type_tag& tag, const suffix_def& suffix, const target_type& base, const prefix_def& prefix = prefix_def());
         target_type(const type_tag& tag, const suffixes_t& suffixes, const prefixes_t& prefixes = prefixes_t());
         target_type(const type_tag& tag, const suffixes_t& suffixes, const target_type& base, const prefixes_t& prefixes = prefixes_t());
         
         const type_tag& tag() const { return tag_; }
         const suffixes_t& suffixes() const { return suffixes_; }
         const prefixes_t& prefixes() const { return prefixes_; }
         const std::string& suffix_for(const std::string& s, const feature_set& environment) const;
         const std::string& suffix_for(const feature_set& environment) const;
         const std::string& prefix_for(const feature_set& environment) const;
         const target_type* base() const { return base_ ; }

         bool equal_or_derived_from(const target_type& rhs) const;
         bool equal_or_derived_from(const type_tag& rhs) const;
         bool operator == (const target_type& rhs) const;
         std::auto_ptr<target_type> clone(const type_registry& tr) const;

			const std::vector<const feature*>&
			valuable_features() const { return valuable_features_; }

      private:
         type_tag tag_;
         suffixes_t suffixes_;
         prefixes_t prefixes_;
         const type_registry* owner_;
         const target_type* base_;
			const std::vector<const feature*> valuable_features_;

         bool equal(const target_type& rhs) const;

			static
			std::vector<const feature*>
			make_valuable_features(const suffixes_t& suffixes,
			                       const prefixes_t& prefixes);
   };
}

#endif //h_911b15ef_ec54_4217_bee8_b61c7584f9f5
