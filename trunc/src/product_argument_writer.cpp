#include "stdafx.h"
#include "product_argument_writer.h"

namespace hammer{

product_argument_writer::product_argument_writer(const std::string& name, 
                                                 const type& t)
   : targets_argument_writer(name, t)
{
}

argument_writer* product_argument_writer::clone() const
{
   return new product_argument_writer(*this);
}

void product_argument_writer::write(std::ostream& output, const build_node& node, const build_environment& environment) const
{
   output << "[product_argument_writer]";
}

}
