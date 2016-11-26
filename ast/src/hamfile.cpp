#include "stdafx.h"
#include <hammer/ast/hamfile.h>
#include <hammer/ast/project_def.h>
#include <hammer/ast/statement.h>
#include <hammer/ast/visitor.h>

namespace hammer{namespace ast{

bool hamfile::accept(visitor& v) const
{
   return v.visit(*this);
}

}}
