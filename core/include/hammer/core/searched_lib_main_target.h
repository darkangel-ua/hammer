#pragma once
#include <hammer/core/main_target.h>

namespace hammer {

class searched_lib_main_target : public main_target {
   public:
      searched_lib_main_target(const basic_meta_target* mt,
                               const std::string& name,
                               const std::string& lib_name,
                               const feature_set* props,
                               const target_type& t);
   protected:
      build_nodes_t generate_impl() const override;

   private:
      const target_type* type_;
      const std::string lib_name_;
};

}
