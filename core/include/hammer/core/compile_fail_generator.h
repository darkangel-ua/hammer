#ifndef HAMMER_CORE_COMPILE_FAIL_GENERATOR_H
#define HAMMER_CORE_COMPILE_FAIL_GENERATOR_H

#include <hammer/core/generator.h>

namespace hammer {

class compile_fail_generator : public generator
{
   public:
      compile_fail_generator(engine& e,
                             std::unique_ptr<generator> compile_generator,
                             std::unique_ptr<build_action> compile_action);
      build_nodes_t
      construct(const target_type& type_to_construct,
                const feature_set& props,
                const build_nodes_t& sources,
                const basic_target* source_target,
                const std::string* composite_target_name,
                const main_target& owner) const override;

   protected:
      basic_target*
      create_target(const main_target* mt,
                    const build_node::sources_t& sources,
                    const std::string& n,
                    const target_type* t,
                    const feature_set* f) const override;

   private:
      std::unique_ptr<generator> compile_generator_;
};

}

#endif
