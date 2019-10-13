#pragma once
#include <unordered_map>
#include <string>
#include <vector>

namespace hammer {

class feature_def;

class subfeature_def {
      friend class feature_def;
   public:
      typedef std::vector<std::string> legal_values_t;

      subfeature_def(const subfeature_def&) = delete;
      subfeature_def(subfeature_def&&) = delete;
      subfeature_def& operator = (const subfeature_def&) = delete;

      const std::string& name() const { return name_; }
      const legal_values_t& legal_values(const std::string& feature_value) const;
      bool is_legal_value(const std::string& feature_value,
                          const std::string& value) const;
      void extend_legal_values(const std::string& feature_value,
                               const std::string& new_legal_value);

   private:
      typedef std::unordered_map<std::string/*feature value*/, legal_values_t> all_legal_values;

      // used only by feature_def
      subfeature_def(const feature_def& owner,
                     const std::string& name);

      const feature_def* owner_;
      const std::string name_;
      all_legal_values all_legal_values_;
};

}
