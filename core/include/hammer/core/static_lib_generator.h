#pragma once
#include <hammer/core/generator.h>

namespace hammer {

class static_lib_generator : public generator {
   public:
      static_lib_generator(hammer::engine& e,
                           const std::string& name,
                           const consumable_types_t& source_types,
                           const producable_types_t& target_types,
                           bool composite,
                           const build_action_ptr& action,
                           const feature_set* c);
      build_nodes_t
      construct(const target_type& type_to_construct,
                const feature_set& props,
                const std::vector<boost::intrusive_ptr<build_node> >& sources,
                const basic_build_target* t,
                const std::string* name,
                const main_target& owner) const override;

   private:
      const target_type& shared_lib_;
      const target_type& static_lib_;
      const target_type& searched_lib_;
};

}
