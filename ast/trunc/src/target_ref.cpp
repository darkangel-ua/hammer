#include "stdafx.h"
#include <hammer/ast/target_ref.h>

namespace hammer{namespace ast{

bool target_ref::accept(visitor& v) const
{
   return v.visit(*this);
}

bool target_ref::has_target_name() const
{
   return target_name_.valid();
}

}}

