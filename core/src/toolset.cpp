#include <stdexcept>
#include <hammer/core/toolset.h>
#include <hammer/core/feature_set.h>

namespace hammer {

toolset::toolset(const std::string& name,
                 const rule_declaration& use_rule)
   : name_(name),
     use_rule_(use_rule)
{
}

void toolset::register_configured(const std::string& version,
                                  const feature_set& constraints,
                                  const YAML::Node configuration_info) {
   if (is_already_configured(version, constraints))
      throw std::runtime_error("toolset " + name() + ", version = " + version + ", constraints = " + dump_for_hash(constraints, true) + " already configured");

   registered_versions_.push_back({version, &constraints, configuration_info});
}

bool toolset::is_already_configured(const std::string& version,
                                    const feature_set& constraints) const {
   for (const auto& v : registered_versions_) {
      if (v.version_ == version && v.constraints_->compatible_with(constraints))
         return true;
   }

   return false;
}

toolset::~toolset() {

}

}
