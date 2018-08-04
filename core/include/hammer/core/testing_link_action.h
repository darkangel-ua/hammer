#pragma once
#include <hammer/core/testing_success_action.h>

namespace hammer {

class testing_link_action : public testing_success_action {
   public:
      testing_link_action(const build_action_ptr& link_action)
         : testing_success_action("testing.link", link_action)
      {}
};

}
