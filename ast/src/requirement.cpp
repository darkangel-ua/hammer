#include "stdafx.h"
#include <hammer/ast/requirement.h>
#include <hammer/ast/visitor.h>
#include <hammer/ast/feature.h>

namespace hammer{ namespace ast{

bool simple_requirement::accept(visitor& v) const
{
   return v.visit(*this);
}

bool conditional_requirement::accept(visitor& v) const
{
   return v.visit(*this);
}

parscore::source_location simple_requirement::start_loc() const
{
   return is_public() ? public_tag() : value_->start_loc();
}

parscore::source_location 
conditional_requirement::start_loc() const 
{ 
   return features_.front()->start_loc(); 
}

}}
