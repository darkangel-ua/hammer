#ifndef HAMMER_CORE_COMPILE_FAIL_GENERATOR_H
#define HAMMER_CORE_COMPILE_FAIL_GENERATOR_H

#include <hammer/core/generator.h>
#include <hammer/core/build_action.h>

namespace hammer {

class compile_fail_generator : public generator
{
   public:
      compile_fail_generator(engine& e,
                             std::unique_ptr<generator> failing_compile_generator);
      build_nodes_t
      construct(const target_type& type_to_construct,
                const feature_set& props,
                const build_nodes_t& sources,
                const basic_build_target* source_target,
                const std::string* composite_target_name,
                const main_target& owner) const override;

   protected:
      basic_build_target*
      create_target(const main_target* mt,
                    const build_node::sources_t& sources,
                    const std::string& n,
                    const target_type* t,
                    const feature_set* f) const override;

   private:
      std::unique_ptr<generator> compile_generator_;
};

class compile_fail_build_action : public build_action
{
   public:
      compile_fail_build_action(engine& e,
		                          const build_action_ptr& compile_action);

		std::string	target_tag(const build_node& node,
		                       const build_environment& environment) const override;
		std::vector<const feature*> valuable_features() const override { return compile_action_->valuable_features(); }

	protected:
      bool execute_impl(const build_node& node,
                        const build_environment& environment) const override;

   private:
      build_action_ptr compile_action_;
      const target_type& output_target_type_;
};

void add_compile_fail_generator(engine& e,
                                std::unique_ptr<generator> compile_generator);

}

#endif
