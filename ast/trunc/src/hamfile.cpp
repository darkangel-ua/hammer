#include "stdafx.h"
#include <hammer/ast/hamfile.h>
#include <hammer/ast/project_def.h>
#include <hammer/ast/statement.h>
#include <hammer/ast/visitor.h>

namespace hammer{namespace ast{

bool hamfile::accept(visitor& v) const
{
   if (v.visit_enter(*this) &&
       project_->accept(v))
   {
      v.visit(statements_);
   }

   return v.visit_leave(*this);
}

}}
