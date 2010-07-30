#include "stdafx.h"
#include <hammer/ast/path_like_seq.h>

namespace hammer{ namespace ast{

bool path_like_seq::accept(visitor& v) const
{
   return v.visit(*this);
}

}}
