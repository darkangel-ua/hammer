#include "stdafx.h"
#include <hammer/core/targets_argument_writer.h>

namespace hammer{

targets_argument_writer::targets_argument_writer(const std::string& name, 
                                                 const target_type& t)
   : argument_writer(name),
     source_type_(&t)
{
}

}
