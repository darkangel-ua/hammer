#include <hammer/core/virtual_project.h>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

namespace hammer {

virtual_project::virtual_project(engine& e,
                                 const project* parent)
   : project(e, parent, location_t("/virtual_projects") / to_string(boost::uuids::random_generator{}()))
{
}

virtual_project::virtual_project(engine& e,
                                 const project* parent,
                                 const std::string& name)
   : virtual_project(e, parent)
{
   this->name(name);
}

virtual_project::virtual_project(engine& e,
                                 const project* parent,
                                 const std::string& name,
                                 const location_t& relative_path)
   : project(e, parent, location_t("/virtual_projects") / relative_path)
{
   this->name(name);
}

}
