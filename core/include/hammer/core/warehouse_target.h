#pragma once
#include <hammer/core/main_target.h>

namespace hammer {

class warehouse_target : public main_target {
   public:
      warehouse_target(const basic_meta_target& mt,
                       const std::string& name,
                       const feature_set& build_request);
      ~warehouse_target();

   protected:
      build_nodes_t generate_impl() const override;
};

// throw from warehouse_target::generate to signal that build tree has some libs to download/install
class warehouse_unresolved_target_exception : public std::runtime_error {
   public:
      warehouse_unresolved_target_exception();
};

}
