#include <hammer/ast/feature_set.h>
#include <hammer/ast/visitor.h>

namespace hammer{ namespace ast{

bool feature_set::accept(visitor& v) const
{
   return v.visit(*this);
}

}}
