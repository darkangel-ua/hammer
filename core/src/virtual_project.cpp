#include <hammer/core/virtual_project.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace hammer {

virtual_project::virtual_project(engine& e)
   : project(e, "/virtual_projects/" + to_string(boost::uuids::random_generator()()))
{
}

virtual_project::virtual_project(engine& e,
                                 const std::string& name)
   : virtual_project(e)
{
   this->name(name);
}

}
