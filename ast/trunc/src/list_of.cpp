#include "stdafx.h"
#include <hammer/ast/list_of.h>

namespace hammer{ namespace ast{

bool list_of::accept(visitor& v) const
{
   return true;
}

}}
