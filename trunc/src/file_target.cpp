#include "stdafx.h"
#include "file_target.h"

namespace hammer
{

std::vector<boost::intrusive_ptr<build_node> > file_target::generate()
{
   return std::vector<boost::intrusive_ptr<build_node> >();
}

}
