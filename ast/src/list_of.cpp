#include "stdafx.h"
#include <hammer/ast/list_of.h>
#include <hammer/ast/visitor.h>
#include <hammer/ast/casts.h>

namespace hammer{ namespace ast{

bool list_of::accept(visitor& v) const
{
   if (v.visit_enter(*this))
   {
      v.visit(values_);
   }

   return v.visit_leave(*this);
}

template<>
const list_of* as<list_of>(const node* v) { return dynamic_cast<const list_of*>(v); }

}}
