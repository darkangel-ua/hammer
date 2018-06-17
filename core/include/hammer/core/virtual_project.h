#pragma once
#include <hammer/core/project.h>

namespace hammer {

class virtual_project : public project {
   public:
      virtual_project(engine* e);
      virtual_project(engine* e,
                      const std::string& name);
};

}
