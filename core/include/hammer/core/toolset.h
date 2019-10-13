#pragma once
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <hammer/core/location.h>
#include <hammer/core/rule_manager.h>

namespace hammer {

class engine;
class feature_set;

class toolset {
      struct registered_version {
         std::string version_;
         const feature_set* constraints_;
         YAML::Node configuration_info_;
      };
      using registered_versions = std::vector<registered_version>;

   public:
      using const_iterator = registered_versions::const_iterator;

		toolset(const std::string& name,
		        const rule_declaration& use_rule);

		const std::string& name() const { return name_; }
      const rule_declaration& use_rule() const { return use_rule_; }

      const_iterator begin() const { return registered_versions_.begin(); }
      const_iterator end() const { return registered_versions_.end(); }
      bool empty() const { return registered_versions_.empty(); }

		// try to autoconfigure all known versions
		virtual
      void autoconfigure(engine& e) = 0;

      // try to configure specific version
		virtual
      void configure(engine& e,
		               const std::string& version) = 0;
		virtual
      ~toolset();

   protected:
      void register_configured(const std::string& version,
                               const feature_set& constraints,
                               const YAML::Node configuration_info);

      // check if we already registered something similar to passed version+contraints
      bool is_already_configured(const std::string& version,
                                 const feature_set& constraints) const;

	private:
		const std::string name_;
		const rule_declaration use_rule_;

      registered_versions registered_versions_;
};

}
