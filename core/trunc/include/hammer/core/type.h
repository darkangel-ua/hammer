#if !defined(h_911b15ef_ec54_4217_bee8_b61c7584f9f5)
#define h_911b15ef_ec54_4217_bee8_b61c7584f9f5

#include <string>
#include <vector>
#include <memory>
#include "type_tag.h"

namespace hammer
{
   class type_registry;
   class type
   {
      public:
         typedef std::vector<std::string> suffixes_t;

         type(const type_tag& tag, const std::string& suffix);
         type(const type_tag& tag, const std::string& suffix, const type& base);
         type(const type_tag& tag, const suffixes_t& suffixes);
         type(const type_tag& tag, const suffixes_t& suffixes, const type& base);
         
         const type_tag& tag() const { return tag_; }
         const suffixes_t& suffixes() const { return suffixes_; }
         const std::string& suffix_for(const std::string& s) const;
         const type* base() const { return base_ ; }

         bool equal_or_derived_from(const type& rhs) const;
         std::auto_ptr<type> clone(const type_registry& tr) const;

      private:
         type_tag tag_;
         suffixes_t suffixes_;
         const type_registry* owner_;
         const type* base_;

         bool equal(const type& rhs) const;
   };
}

#endif //h_911b15ef_ec54_4217_bee8_b61c7584f9f5
