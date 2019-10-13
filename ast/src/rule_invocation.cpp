#include <hammer/ast/visitor.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/rule_invocation.h>

namespace hammer { namespace ast {

bool rule_invocation::accept(visitor& v) const
{
   return v.visit(*this);
}

template<>
bool is_a<rule_invocation>(const node& v) { return dynamic_cast<const rule_invocation*>(&v); }

template<>
const rule_invocation* as<rule_invocation>(const node* v) { return dynamic_cast<const rule_invocation*>(v); }

}}
