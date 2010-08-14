#include "stdafx.h"
#include <hammer/ast/expression.h>

namespace hammer{ namespace ast{

bool empty_expr::accept(visitor& v) const
{
   return v.visit(*this);
}

bool id_expr::accept(visitor& v) const
{
   return v.visit(*this);
}

bool named_expr::accept(visitor& v) const
{
   return v.visit(*this);
}

}}
