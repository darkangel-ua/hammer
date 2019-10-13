#include <hammer/ast/casts.h>
#include <hammer/ast/visitor.h>
#include <hammer/ast/statement.h>

namespace hammer { namespace ast {

bool expression_statement::accept(visitor& v) const
{
   return v.visit(*this);
}

template<>
bool is_a<expression_statement>(const node& v) { return dynamic_cast<const expression_statement*>(&v); }

template<>
const expression_statement* as<expression_statement>(const node* v) { return dynamic_cast<const expression_statement*>(v); }

}}

