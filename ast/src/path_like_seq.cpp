#include "stdafx.h"
#include <hammer/ast/path_like_seq.h>
#include <hammer/ast/visitor.h>
#include <hammer/ast/casts.h>

namespace hammer{ namespace ast{

bool path_like_seq::accept(visitor& v) const
{
   return v.visit(*this);
}

parscore::identifier path_like_seq::to_identifier() const
{
   return parscore::identifier(first_.start_lok(), last_.end() - first_.begin());
}

bool path_like_seq::is_simple() const
{
   return first_ == last_;
}

template<>
const path_like_seq* as<path_like_seq>(const node* v) { return dynamic_cast<const path_like_seq*>(v); }

}}
