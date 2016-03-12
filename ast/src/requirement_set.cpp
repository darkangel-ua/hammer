#include "stdafx.h"
#include <hammer/ast/requirement_set.h>
#include <hammer/ast/requirement.h>
#include <hammer/ast/visitor.h>

namespace hammer{ namespace ast{

requirement_set::requirement_set(const requirements_t& requirements) 
   : requirements_(requirements) 
{}

parscore::source_location requirement_set::start_loc() const
{
   return requirements_.front()->start_loc();
}

bool requirement_set::accept(visitor& v) const
{
   if (v.visit_enter(*this))
      v.visit(requirements_);

   return v.visit_leave(*this);
}

}}
