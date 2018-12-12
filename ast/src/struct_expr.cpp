#include <hammer/ast/struct_expr.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/visitor.h>

namespace hammer { namespace ast {

bool struct_expr::accept(visitor& v) const
{
   return v.visit(*this);
}

template<>
bool is_a<struct_expr>(const node& v) { return dynamic_cast<const struct_expr*>(&v); }

template<>
const struct_expr* as<struct_expr>(const node* v) { return dynamic_cast<const struct_expr*>(v); }

}}
