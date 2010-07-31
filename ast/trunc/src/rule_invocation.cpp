#include "stdafx.h"
#include <hammer/ast/rule_invocation.h>

namespace hammer{namespace ast{

bool rule_invocation::accept(visitor& v) const
{
   if (v.visit_enter(*this))
   {
      v.visit(arguments_);
   }

   return v.visit_leave(*this);
}

}}