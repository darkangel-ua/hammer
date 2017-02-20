#if !defined(h_655d0ae9_850d_4f01_a912_19b32eaf223e)
#define h_655d0ae9_850d_4f01_a912_19b32eaf223e

#include "feature_base.h"

namespace hammer
{
   class subfeature;
   class feature_registry;

   class feature : public feature_base
   {
      public:
         friend class feature_registry;
         typedef std::vector<const subfeature*> subfeatures_t;

         const feature_def& definition() const { return *static_cast<const feature_def*>(definition_); }
         // FIXME: will not work when rhs and lhs from different feature_registries
         const subfeature* find_subfeature(const subfeature& v) const;
         const subfeature* find_subfeature(const std::string& v) const;
         const subfeatures_t& subfeatures() const { return subfeatures_; }

         // <toolset>gcc will be equal only to <toolset>gcc - full match, including subfeatures
         bool operator == (const feature& rhs) const;

         // if this == <toolset>gcc-6 then it will contain f == <toolset>gcc AND f == <toolset>gcc-6, partial match
         bool contains(const feature& f) const;

         bool operator != (const feature& rhs) const { return !(*this == rhs); }
         bool operator < (const feature& rhs) const;

      private:
         subfeatures_t subfeatures_;

         feature(const feature_def* def, const std::string& value);
         feature(const feature_def* def, const std::string& value, const subfeatures_t& subfeatures);

         bool equal_without_subfeatures(const feature& rhs) const;
   };
}

#endif //h_655d0ae9_850d_4f01_a912_19b32eaf223e
