#include <hammer/core/testing_fail_action.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/testing_build_environment.h>
#include <hammer/core/product_argument_writer.h>

namespace hammer {

testing_fail_action::testing_fail_action(engine& e,
                                         const std::string& name,
                                         const build_action_ptr& fail_action,
                                         const type_tag& output_target_type)
   : build_action(name + "(" + fail_action->name() + ")"),
     fail_action_(fail_action),
     output_target_type_(e.get_type_registry().get(output_target_type))
{}

std::vector<const feature*>
testing_fail_action::valuable_features() const
{
   return fail_action_->valuable_features();
}

std::string
testing_fail_action::target_tag(const build_node& node,
                                const build_environment& environment) const
{
   return fail_action_->target_tag(node, environment);
}

bool testing_fail_action::execute_impl(const build_node& node,
                                       const build_environment& environment) const
{
   testing_build_environment env(environment);

   product_argument_writer output_product_builder("testing-fail-output", output_target_type_);
   std::stringstream product_path_stream;
   output_product_builder.write(product_path_stream, node, env);

   const bool result = !fail_action_->execute(node, env);
   env.write_tag_file(product_path_stream.str(), env.output_stream_.str() + env.error_stream_.str());

   return result;
}

}
