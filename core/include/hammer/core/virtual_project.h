#pragma once
#include <hammer/core/project.h>

namespace hammer {

class virtual_project : public project {
   public:
      virtual_project(engine& e,
                      const project* parent);
      virtual_project(engine& e,
                      const project* parent,
                      const std::string& name);
};

}
