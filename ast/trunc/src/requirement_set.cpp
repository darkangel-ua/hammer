#include "stdafx.h"
#include <hammer/ast/requirement_set.h>
#include <hammer/ast/visitor.h>

namespace hammer{ namespace ast{

bool requirement_set::accept(visitor& v) const
{
   if (v.visit_enter(*this))
      v.visit(requirements_);

   return v.visit_leave(*this);
}

}}
