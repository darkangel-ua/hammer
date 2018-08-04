#pragma once
#include <hammer/core/testing_success_action.h>

namespace hammer {

class testing_compile_action : public testing_success_action {
   public:
      testing_compile_action(const build_action_ptr& compile_action)
         : testing_success_action("testing.compile", compile_action)
      {}
};

}
