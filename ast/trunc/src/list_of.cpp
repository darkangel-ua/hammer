#include "stdafx.h"
#include <hammer/ast/list_of.h>

namespace hammer{ namespace ast{

bool list_of::accept(visitor& v) const
{
   if (v.visit_enter(*this))
   {
      v.visit(values_);
   }

   return v.visit_leave(*this);
}

}}
