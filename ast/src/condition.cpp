#include <hammer/ast/condition.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/visitor.h>

namespace hammer{namespace ast{

template<>
bool is_a<condition_expr>(const node& v) { return dynamic_cast<const condition_expr*>(&v); }

template<>
const condition_expr* as<condition_expr>(const node* v) { return dynamic_cast<const condition_expr*>(v); }

bool logical_or::accept(visitor& v) const
{
   return v.visit(*this);
}

bool logical_and::accept(visitor& v) const
{
   return v.visit(*this);
}

bool condition_expr::accept(visitor& v) const
{
   return v.visit(*this);
}

template<>
const logical_or* as<logical_or>(const node* v) { return dynamic_cast<const logical_or*>(v); }

template<>
const logical_and* as<logical_and>(const node* v) { return dynamic_cast<const logical_and*>(v); }

}}
