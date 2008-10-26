#include "stdafx.h"
#include "pch_main_target.h"
#include <cassert>

namespace hammer
{

std::vector<boost::intrusive_ptr<build_node> > 
pch_main_target::generate()
{
   std::vector<boost::intrusive_ptr<hammer::build_node> > result(main_target::generate());
   return result;
}

}