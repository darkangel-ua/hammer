#pragma once
#include <hammer/core/source_target.h>

namespace hammer {

class htmpl_source_target : public source_target
{
   public:
      htmpl_source_target(const main_target* mt,
                          const location_t& l,
                          const std::string& name,
                          const target_type* t,
                          const feature_set* f)
         : source_target(mt, l, name, t, f)
      {}

      build_nodes_t
      generate() const override;
};

}
