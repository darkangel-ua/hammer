#pragma once
#include <hammer/core/main_target.h>

namespace hammer {

class testing_main_target : public main_target {
   public:
      testing_main_target(const basic_meta_target* mt,
                          const std::string& name,
                          const target_type* t,
                          const feature_set* props);
   protected:
      build_nodes_t
      generate_impl() const override;
};

}
