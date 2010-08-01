#include "stdafx.h"
#include <hammer/ast/feature_set.h>

namespace hammer{ namespace ast{

bool feature_set::accept(visitor& v) const
{
   if (v.visit_enter(*this))
      v.visit(features_);

   return v.visit_leave(*this);
}

}}
