#include "stdafx.h"
#include <hammer/ast/feature.h>

namespace hammer{namespace ast{

bool feature::accept(visitor& v) const
{
   return v.visit(*this);
}

}}
