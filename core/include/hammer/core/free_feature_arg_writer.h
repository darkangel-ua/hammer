#pragma once
#include <hammer/core/argument_writer.h>

namespace hammer {

class feature_set;
class feature_def;
class feature_registry;

class free_feature_arg_writer : public argument_writer {
   public:
      free_feature_arg_writer(const std::string& name,
                              feature_registry& fr,
                              const std::string& feature_name,
                              const std::string& prefix = std::string(),
                              const std::string& suffix = std::string(),
                              const std::string& delimiter = std::string(" "),
                              const std::string& global_prefix = std::string(),
                              const std::string& global_suffix = std::string());
      free_feature_arg_writer* clone() const override;
      std::vector<const feature*> valuable_features() const override;

   private:
      const feature_def& feature_def_;
      feature_registry& fr_;
      std::string prefix_;
      std::string suffix_;
      std::string delimiter_;
      std::string global_prefix_;
      std::string global_suffix_;

      void write_impl(std::ostream& output,
                      const build_node& node,
                      const build_environment& environment) const override;
};

}
