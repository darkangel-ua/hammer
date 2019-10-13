#include <hammer/ast/visitor.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/feature.h>

namespace hammer { namespace ast {

bool feature::accept(visitor& v) const
{
   return v.visit(*this);
}

template<>
bool is_a<feature>(const node& v) { return dynamic_cast<const feature*>(&v); }

template<>
const feature* as<feature>(const node* v) { return dynamic_cast<const feature*>(v); }

}}
