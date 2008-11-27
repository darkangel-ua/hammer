#if !defined(h_655d0ae9_850d_4f01_a912_19b32eaf223e)
#define h_655d0ae9_850d_4f01_a912_19b32eaf223e

#include "feature_base.h"

namespace hammer
{
   class subfeature;
   class feature : public feature_base
   {
      public:
         typedef std::vector<const subfeature*> subfeatures_t;

         feature(const feature_def* def, const pstring& value);
         feature(const feature_def* def, const pstring& value, const subfeatures_t& subfeatures);
         const feature_def& definition() const { return *static_cast<const feature_def*>(definition_); }
         const subfeature* find_subfeature(const subfeature& v) const;
         const subfeature* find_subfeature(const std::string& v) const;
         const subfeatures_t subfeatures() const { return subfeatures_; }

         bool operator == (const feature& rhs) const;
         bool operator < (const feature& rhs) const;

      private:
         subfeatures_t subfeatures_;
   };
}

#endif //h_655d0ae9_850d_4f01_a912_19b32eaf223e
