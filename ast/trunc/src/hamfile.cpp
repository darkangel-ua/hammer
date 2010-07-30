#include "stdafx.h"
#include <hammer/ast/hamfile.h>
#include <hammer/ast/project_def.h>
#include <hammer/ast/statement.h>

namespace hammer{namespace ast{

bool hamfile::accept(visitor& v) const
{
   if (v.visit_enter(*this) &&
       project_->accept(v))
   {
      for(statements_t::const_iterator i = statements_.begin(), last = statements_.end(); i != last; ++i)
         if (!(**i).accept(v))
            break;
   }

   return v.visit_leave(*this);
}

}}
