#include "stdafx.h"
#include <hammer/ast/expression.h>
#include <hammer/ast/visitor.h>
#include <hammer/ast/casts.h>

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

bool public_expr::accept(visitor& v) const
{
   return v.visit(*this);
}

bool is_error_expr(const expression* e)
{
   return dynamic_cast<const error_expression*>(e);
}

template<>
bool is_a<error_expression>(const node& v) { return dynamic_cast<const error_expression*>(&v); }

template<>
bool is_a<named_expr>(const node& v) { return dynamic_cast<const named_expr*>(&v); }

template<>
bool is_a<public_expr>(const node& v) { return dynamic_cast<const public_expr*>(&v); }

template<>
bool is_a<empty_expr>(const node& v) { return dynamic_cast<const empty_expr*>(&v); }

template<>
bool is_a<id_expr>(const node& v) { return dynamic_cast<const id_expr*>(&v); }

template<>
const named_expr& as<named_expr>(const node& v) { return dynamic_cast<const named_expr&>(v); }

template<>
const public_expr& as<public_expr>(const node& v) { return dynamic_cast<const public_expr&>(v); }

template<>
const public_expr* as<public_expr>(const node* v) { return dynamic_cast<const public_expr*>(v); }

template<>
const empty_expr& as<empty_expr>(const node& v) { return dynamic_cast<const empty_expr&>(v); }

template<>
const empty_expr* as<empty_expr>(const node* v) { return dynamic_cast<const empty_expr*>(v); }

template<>
const id_expr* as<id_expr>(const node* v) { return dynamic_cast<const id_expr*>(v); }

}}
