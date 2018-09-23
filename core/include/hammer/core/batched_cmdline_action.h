#pragma once
#include <hammer/core/cmdline_action.h>

namespace hammer {

class batched_cmdline_action : public cmdline_action {
   public:
      batched_cmdline_action(const std::string& name);

   protected:
      bool execute_impl(const build_node& node,
                        const build_environment& environment) const override;
};

}
