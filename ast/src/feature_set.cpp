#include <hammer/ast/feature_set.h>
#include <hammer/ast/visitor.h>
#include <hammer/ast/casts.h>

namespace hammer{ namespace ast{

bool feature_set::accept(visitor& v) const
{
   return v.visit(*this);
}

template<>
bool is_a<feature_set>(const node& v) { return dynamic_cast<const feature_set*>(&v); }

template<>
const feature_set* as<feature_set>(const node* v) { return dynamic_cast<const feature_set*>(v); }

}}
