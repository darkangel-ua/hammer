#include <hammer/ast/hamfile.h>
#include <hammer/ast/visitor.h>

namespace hammer{namespace ast{

bool hamfile::accept(visitor& v) const
{
   return v.visit(*this);
}

}}
