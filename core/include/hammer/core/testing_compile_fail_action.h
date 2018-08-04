#pragma once
#include <hammer/core/testing_fail_action.h>
#include <hammer/core/types.h>

namespace hammer {

class testing_compile_fail_action : public testing_fail_action {
   public:
      testing_compile_fail_action(engine& e,
                                  const build_action_ptr& fail_action)
         : testing_fail_action(e, "testing.compile-fail", fail_action, types::TESTING_OUTPUT)
      {}
};

}
