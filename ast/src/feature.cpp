#include "stdafx.h"
#include <hammer/ast/feature.h>
#include <hammer/ast/visitor.h>

namespace hammer{namespace ast{

bool feature::accept(visitor& v) const
{
   return v.visit(*this);
}

}}
