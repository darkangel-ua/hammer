#pragma once
#include <string>

namespace hammer {

class subfeature_def;

class subfeature {
   public:
      subfeature(const subfeature_def& def,
                 const std::string& value);
      ~subfeature();

      const subfeature_def& definition() const { return *definition_; }
      const std::string& name() const;
      const std::string& value() const { return value_; }

      // FIXME: wrong results if we compare subfeatures from different feature_registries
      bool operator == (const subfeature& rhs) const;
      bool operator != (const subfeature& rhs) const { return !(*this == rhs); }

   private:
      const subfeature_def* definition_;
      const std::string value_;
};

}
