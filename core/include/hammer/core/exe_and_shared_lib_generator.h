#pragma once
#include <hammer/core/generator.h>

namespace hammer {

class exe_and_shared_lib_generator : public generator {
   public:
      exe_and_shared_lib_generator(hammer::engine& e,
                                   const std::string& name,
                                   const consumable_types_t& source_types,
                                   const producable_types_t& target_types,
                                   bool composite,
                                   const build_action_ptr& action,
                                   const feature_set* constraints,
                                   const feature_set* additional_target_properties);

      build_nodes_t
      construct(const target_type& type_to_construct,
                const feature_set& props,
                const std::vector<boost::intrusive_ptr<build_node> >& sources,
                const basic_build_target* t,
                const std::string* name,
                const main_target& owner) const override;

   protected:
      basic_build_target*
      create_target(const main_target* mt,
                    const build_node::sources_t& sources,
                    const std::string* composite_target_name,
                    const produced_type& type,
                    const feature_set* target_properties) const override;

   private:
      const target_type& searched_lib_;
      const feature_set* additional_target_properties_;
};

}
