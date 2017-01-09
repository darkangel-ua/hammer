#pragma once
#include <hammer/core/project.h>

namespace hammer {

class warehouse_project : public project
{
   public:
      warehouse_project(engine& e,
                        const pstring& name,
                        const location_t& location);
      ~warehouse_project();
};

}
