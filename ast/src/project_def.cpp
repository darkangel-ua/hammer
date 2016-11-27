#include "stdafx.h"
#include <hammer/ast/project_def.h>
#include <hammer/ast/visitor.h>

namespace hammer{namespace ast{

project_def::project_def(const parscore::identifier& rule_name,
                         const expressions_t& arguments)
   : rule_invocation(rule_name, arguments)
{

}

bool project_def::accept(visitor& v) const
{
   return v.visit(*this);
}

}}
