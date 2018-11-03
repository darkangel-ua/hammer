#include <hammer/core/virtual_project.h>
#include <boost/guid.hpp>

namespace hammer {

virtual_project::virtual_project(engine& e,
                                 const project* parent)
   : project(e, parent, "/virtual_projects/" + boost::guid::create().to_string())
{
}

virtual_project::virtual_project(engine& e,
                                 const project* parent,
                                 const std::string& name)
   : virtual_project(e, parent)
{
   this->name(name);
}

}
