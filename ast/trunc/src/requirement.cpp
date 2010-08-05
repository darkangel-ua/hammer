#include "stdafx.h"
#include <hammer/ast/requirement.h>

namespace hammer{ namespace ast{

bool simple_requirement::accept(visitor& v) const
{
   return v.visit(*this);
}

bool conditional_requirement::accept(visitor& v) const
{
   return v.visit(*this);
}

}}
