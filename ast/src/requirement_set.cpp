#include "stdafx.h"
#include <hammer/ast/requirement_set.h>
#include <hammer/ast/visitor.h>
#include <hammer/ast/casts.h>

namespace hammer{ namespace ast{

requirement_set::requirement_set(const expression* requirements)
   : requirements_(requirements) 
{}

parscore::source_location requirement_set::start_loc() const
{
   return requirements_->start_loc();
}

bool requirement_set::accept(visitor& v) const
{
   return v.visit(*this);
}

template<>
bool is_a<requirement_set>(const node& v) { return dynamic_cast<const requirement_set*>(&v); }

}}

