#include <hammer/core/virtual_project.h>
#include <boost/guid.hpp>

namespace hammer {

virtual_project::virtual_project(engine& e)
   : project(e, "/virtual_projects/" + boost::guid::create().to_string())
{
}

virtual_project::virtual_project(engine& e,
                                 const std::string& name)
   : virtual_project(e)
{
   this->name(name);
}

}
