#include <hammer/core/compile_fail_generator.h>
#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/virtual_build_target.h>
#include <hammer/core/target_type.h>
#include <hammer/core/build_action.h>
#include <hammer/core/proxied_build_environment.h>
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

class compile_fail_build_action : public build_action
{
   public:
      compile_fail_build_action(unique_ptr<build_action> compile_action,
                                const target_type& output_target_type)
         : build_action("compile-fail.action"),
           compile_action_(move(compile_action)),
           output_target_type_(output_target_type)
      {}

      string target_tag(const build_node& node,
                        const build_environment& environment) const override
      {
         return compile_action_->target_tag(node, environment);
      }

   protected:
      bool execute_impl(const build_node& node,
                        const build_environment& environment) const override
      {
         compile_fail_build_environment env(environment);

         product_argument_writer output_product_builder("compile-fail-output", output_target_type_);
         stringstream product_path_stream;
         output_product_builder.write(product_path_stream, node, env);

         const bool result = !compile_action_->execute(node, env);
         env.write_tag_file(product_path_stream.str(), env.output_stream_.str() + env.error_stream_.str());

         return result;
      }

   private:
      unique_ptr<build_action> compile_action_;
      const target_type& output_target_type_;
};

compile_fail_generator::compile_fail_generator(engine& e,
                                               unique_ptr<generator> compile_generator,
                                               unique_ptr<build_action> compile_action)
   : generator(e,
               "compile-fail." + compile_generator->name(),
               compile_generator->consumable_types(),
               make_product_types(e, {types::TESTING_OUTPUT, types::TESTING_COMPILE_FAIL}),
               true,
               compile_generator->constraints()),
     compile_generator_(move(compile_generator))
{
   const target_type& output_type = e.get_type_registry().get(types::TESTING_OUTPUT);
   unique_ptr<compile_fail_build_action> compile_fail_action(new compile_fail_build_action(move(compile_action), output_type));
   compile_generator_->action(move(compile_fail_action));
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
                                      const string& n,
                                      const target_type* t,
                                      const feature_set* f) const
{
   if (get_engine().get_type_registry().get(types::TESTING_COMPILE_FAIL) == *t)
      return new virtual_build_target(mt, n, t, f, false);

   return generator::create_target(mt, sources, n, t, f);
}

}
