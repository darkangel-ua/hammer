#include <hammer/core/warehouse_project.h>
#include <hammer/core/requirements_decl.h>

namespace hammer {

warehouse_project::warehouse_project(engine& e,
                                     const std::string& name,
                                     const location_t& location)
   : project(e, nullptr, name, location, {}, {}, {})
{

}

warehouse_project::~warehouse_project()
{

}

}
