#include "stdafx.h"
#include <hammer/ast/sources.h>
#include <hammer/ast/visitor.h>

namespace hammer{namespace ast{

bool sources::accept(visitor& v) const
{ 
   return v.visit(*this);
}

}}

