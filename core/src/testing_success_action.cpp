#include <hammer/core/testing_build_environment.h>
#include <hammer/core/testing_success_action.h>

namespace hammer {

testing_success_action::testing_success_action(const std::string& name,
                                               const build_action_ptr& success_action)
   : build_action(name + "(" + success_action->name() + ")"),
     success_action_(success_action)
{}

std::string
testing_success_action::target_tag(const build_node& node,
                                   const build_environment& environment) const
{
   return success_action_->target_tag(node, environment);
}

bool testing_success_action::execute_impl(const build_node& node,
                                          const build_environment& environment) const
{
   testing_build_environment env(environment);

   return success_action_->execute(node, env);
}

}
