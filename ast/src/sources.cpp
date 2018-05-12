#include <hammer/ast/visitor.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/sources.h>

namespace hammer { namespace ast {

bool sources::accept(visitor& v) const
{ 
   return v.visit(*this);
}

template<>
bool is_a<sources>(const node& v) { return dynamic_cast<const sources*>(&v); }

template<>
const sources* as<sources>(const node* v) { return dynamic_cast<const sources*>(v); }

}}

