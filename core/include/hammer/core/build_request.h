#pragma once
#include <string>
#include <list>

namespace hammer {

class feature;
class feature_set;
class feature_registry;
class feature_ref;

class build_request {
   public:
      struct unresolved_feature {
         std::string name_;
         std::string value_;
      };

      using unresolved_features = std::list<unresolved_feature>;

      explicit build_request(const feature_registry& fr);
      explicit build_request(const feature_set& resolved_features);

      ~build_request();

      // return true if there is no unresolved features
      bool resolved_completely() const { return unresolved_features_.empty(); }

      // return new build_request with all possible features resolved
      // that means this newly created build_request might still has some unresolveds
      build_request
      resolve(const feature_registry& context) const;

      // return already resoved properties
      // there still might be unresolved ones
      feature_set&
      resolved_request() { return *resolved_features_; }

      const feature_set&
      resolved_request() const { return *resolved_features_; }

      // will join with resolved set
      void join(const feature_set& props);
      void join(feature_ref f);
      void join_unresolved(std::string name,
                           std::string value);

      std::string
      string(const bool full = false) const;

   private:
      feature_set* resolved_features_;
      unresolved_features unresolved_features_;
};

}
