#pragma once
#include <hammer/core/generator.h>

namespace hammer {

class testing_compile_link_base_generator : public generator {
   public:
      testing_compile_link_base_generator(engine& e,
                                          const std::string& name,
                                          std::unique_ptr<generator> compile_generator,
                                          const generator::producable_types_t& products,
                                          const type_tag& virtual_target_type);
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
                    const std::string* target_name,
                    const produced_type& type,
                    const feature_set* f) const override;
   private:
      std::unique_ptr<generator> compile_generator_;
      const target_type& virtual_target_type_;
};

}
