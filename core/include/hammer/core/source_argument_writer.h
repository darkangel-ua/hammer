#pragma once
#include <hammer/core/targets_argument_writer.h>

namespace hammer {

class basic_build_target;

class source_argument_writer : public targets_argument_writer {
   public:
      enum output_strategy{RELATIVE_TO_MAIN_TARGET, FULL_PATH, RELATIVE_TO_WORKING_DIR, WITHOUT_PATH};

      source_argument_writer(const std::string& name,
                             const target_type& t,
                             bool exact_type = true,
                             output_strategy os = RELATIVE_TO_MAIN_TARGET,
                             const std::string& quoting_string = "\"",
                             const std::string& prefix = std::string());
      argument_writer* clone() const override;

   protected:
      void write_impl(std::ostream& output,
                      const build_node& node,
                      const build_environment& environment) const override;

      // return true if source type is accepted for output
      bool accept(const basic_build_target& source) const;

   private:
      bool exact_type_;
      output_strategy output_strategy_;
      std::string quoting_string_;
      std::string prefix_;
};

}
