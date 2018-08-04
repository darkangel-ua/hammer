#pragma once
#include <iosfwd>
#include "build_node.h"

namespace hammer {

class project;
class build_environment;

class builder {
   public:
      struct result {
         unsigned updated_targets_ = 0;
         unsigned failed_to_build_targets_ = 0;
         unsigned skipped_targets_ = 0;
      };

      builder(const build_environment& environment,
              volatile bool& interrupt_flag,
              unsigned worker_count,
              bool unconditional_build);
      ~builder();

      result build(build_nodes_t& nodes,
                   const project* bounds = nullptr);
      result build(build_node& node,
                   const project* bounds = nullptr);
      static
      void generate_graphviz(std::ostream& os,
                             const build_nodes_t& nodes,
                             const project* bounds = nullptr);
   private:
      struct impl_t;
      impl_t* impl_;
};

}
