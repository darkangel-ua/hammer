#pragma once
#include <hammer/core/main_target.h>

namespace hammer {

class copy_main_target : public main_target {
   public:
      copy_main_target(const basic_meta_target* mt,
                       const std::string& name,
                       const feature_set* props);

   protected:
      bool need_signature() const override { return false; }
      build_nodes_t generate_impl() const override;
};

}
