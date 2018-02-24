#include <hammer/core/compile_fail_generator.h>
#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/virtual_build_target.h>
#include <hammer/core/target_type.h>
#include <hammer/core/build_action.h>
#include <hammer/core/proxied_build_environment.h>
#include <hammer/core/generator_registry.h>
#include "buffered_output_environment.h"
#include <hammer/core/product_argument_writer.h>

using namespace std;

namespace hammer {

class compile_fail_build_environment : public proxied_build_environment
{
   public:
      compile_fail_build_environment(const build_environment& env) : proxied_build_environment(env) {}

      std::ostream& output_stream() const override
      {
         return output_stream_;
      }

      std::ostream& error_stream() const override
      {
         return error_stream_;
      }

      mutable std::ostringstream output_stream_;
      mutable std::ostringstream error_stream_;
};

compile_fail_build_action::compile_fail_build_action(engine& e,
                                                     const build_action_ptr& compile_action)
   : build_action("compile-fail.action"),
     compile_action_(compile_action),
     output_target_type_(e.get_type_registry().get(types::TESTING_OUTPUT))
{
}

string
compile_fail_build_action::target_tag(const build_node& node,
                                      const build_environment& environment) const
{
   return compile_action_->target_tag(node, environment);
}

bool compile_fail_build_action::execute_impl(const build_node& node,
                                             const build_environment& environment) const
{
   compile_fail_build_environment env(environment);

   product_argument_writer output_product_builder("compile-fail-output", output_target_type_);
   stringstream product_path_stream;
   output_product_builder.write(product_path_stream, node, env);

   const bool result = !compile_action_->execute(node, env);
   env.write_tag_file(product_path_stream.str(), env.output_stream_.str() + env.error_stream_.str());

   return result;
}

compile_fail_generator::compile_fail_generator(engine& e,
                                               unique_ptr<generator> failing_compile_generator)
   : generator(e,
               "compile-fail." + failing_compile_generator->name(),
               failing_compile_generator->consumable_types(),
               make_product_types(e, {types::TESTING_OUTPUT, types::TESTING_COMPILE_FAIL}),
               true,
               build_action_ptr{},
               failing_compile_generator->constraints()),
     compile_generator_(move(failing_compile_generator))
{
}

build_nodes_t
compile_fail_generator::construct(const target_type& type_to_construct,
                                  const feature_set& props,
                                  const build_nodes_t& sources,
                                  const basic_build_target* source_target,
                                  const std::string* composite_target_name,
                                  const main_target& owner) const
{
   build_nodes_t compile_nodes = compile_generator_->construct(type_to_construct, props, sources, source_target, composite_target_name, owner);
   // assume compile can produce only one node - can't imagine situations with many, right now
   assert(compile_nodes.size() == 1);

   const build_nodes_t this_nodes = generator::construct(type_to_construct, props, sources, source_target, composite_target_name, owner);
   // we always produce only one node
   assert(this_nodes.size() == 1);

   compile_nodes.front()->products_.insert(compile_nodes.front()->products_.end(),
                                           this_nodes.front()->products_.begin(),
                                           this_nodes.front()->products_.end());

   return compile_nodes;
}

basic_build_target*
compile_fail_generator::create_target(const main_target* mt,
                                      const build_node::sources_t& sources,
                                      const string* target_name,
                                      const produced_type& type,
                                      const feature_set* f) const
{
   if (get_engine().get_type_registry().get(types::TESTING_COMPILE_FAIL) == *type.type_)
      return new virtual_build_target(mt, *target_name, type.type_, f, false);

   return generator::create_target(mt, sources, target_name, type, f);
}

void add_compile_fail_generator(engine& e,
                                unique_ptr<generator> compile_generator)
{
   unique_ptr<generator> g(new compile_fail_generator(e, std::move(compile_generator)));
   e.generators().insert(std::move(g));
}

}

