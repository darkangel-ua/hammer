#include <hammer/core/testing_fail_action.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/testing_build_environment.h>
#include <hammer/core/product_argument_writer.h>

namespace hammer {

testing_fail_action::testing_fail_action(engine& e,
                                         const std::string& name,
                                         build_action_ptr failing_action)
   : build_action(name + "(" + failing_action->name() + ")"),
     failing_action_(std::move(failing_action))
{}

std::vector<const feature*>
testing_fail_action::valuable_features() const
{
   return failing_action_->valuable_features();
}

std::string
testing_fail_action::target_tag(const build_node& node,
                                const build_environment& environment) const
{
   return failing_action_->target_tag(node, environment);
}

bool testing_fail_action::execute_impl(const build_node& node,
                                       const build_environment& environment) const
{
   return !failing_action_->execute(node, environment, true);
}

}
