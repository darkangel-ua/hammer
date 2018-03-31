#pragma once
#include <hammer/core/build_node.h>

namespace hammer {

class engine;
class build_environment;

struct cleaner {
   struct result {
      unsigned cleaned_target_count_ = 0;
      unsigned failed_to_clean_target_count_ = 0;
   };

   static
   result clean_all(const build_environment& environment,
                    build_nodes_t& nodes);
};

}
