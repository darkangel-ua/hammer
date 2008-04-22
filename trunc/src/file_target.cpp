#include "stdafx.h"
#include "file_target.h"

namespace hammer
{

boost::intrusive_ptr<build_node> file_target::generate()
{
   return boost::intrusive_ptr<build_node>();
}

}
