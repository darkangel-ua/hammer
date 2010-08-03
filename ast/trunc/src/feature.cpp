#include "stdafx.h"
#include <hammer/ast/feature.h>

namespace hammer{ namespace ast{

bool simple_feature::accept(visitor& v) const
{
   return v.visit(*this);
}

bool conditional_feature::accept(visitor& v) const
{
   return v.visit(*this);
}

}}
