#pragma once
#include <hammer/core/cmdline_action.h>

namespace hammer {

class testing_run_action : public cmdline_action {
   public:
      template<typename T>
      testing_run_action(const std::string& name,
                         boost::shared_ptr<T>& target_writer,
                         boost::shared_ptr<T>& output_writer)
      : cmdline_action(name, target_writer),
        output_writer_(boost::static_pointer_cast<argument_writer>(output_writer))
      {
      }

   protected:
      bool run_shell_commands(const std::vector<std::string>& commands,
                              const build_node& node,
                              const build_environment& environment) const override;

   private:
      boost::shared_ptr<argument_writer> output_writer_;
};

}
