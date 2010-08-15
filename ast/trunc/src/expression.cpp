#include "stdafx.h"
#include <hammer/ast/expression.h>
#include <hammer/ast/visitor.h>

namespace hammer{ namespace ast{

bool error_expression::accept(visitor& v) const
{
   return v.visit(*this);
}

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

bool is_error_expr(const expression* e)
{
   return dynamic_cast<const error_expression*>(e);
}

}}
