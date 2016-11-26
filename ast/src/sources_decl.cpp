#include "stdafx.h"
#include <hammer/ast/sources_decl.h>
#include <hammer/ast/visitor.h>

namespace hammer{namespace ast{

bool sources_decl::accept(visitor& v) const
{ 
   return v.visit(*this);
}

}}

