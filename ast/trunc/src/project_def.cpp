#include "stdafx.h"
#include <hammer/ast/project_def.h>
#include <hammer/ast/visitor.h>

namespace hammer{namespace ast{

explicit_project_def::explicit_project_def(const parscore::identifier& rule_name,
                                           const expressions_t& arguments)
   : rule_invocation(rule_name, arguments)
{

}

bool explicit_project_def::accept(visitor& v) const
{
   if (v.visit_enter(*this))
   {
      v.visit(arguments());
   }

   return v.visit_leave(*this);
}

bool implicit_project_def::accept(visitor& v) const
{
   return v.visit(*this);
}

}}
