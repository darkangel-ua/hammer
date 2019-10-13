#pragma once
#include <hammer/core/basic_target.h>

namespace hammer {

class main_target;
class target_type;
class engine;
class feature_set;

class source_target : public basic_target {
   public:
      source_target(const main_target* mt,
                    const location_t& l,
                    const std::string& name,
                    const target_type* t,
                    const feature_set* f);

      const location_t& location() const override { return location_; }

   protected:
      build_nodes_t generate_impl() const override;

   private:
      const location_t location_;
};

}
