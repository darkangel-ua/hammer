#include <hammer/ast/casts.h>
#include <hammer/ast/visitor.h>
#include <hammer/ast/rule_invocation.h>
#include <hammer/ast/target_def.h>

namespace hammer { namespace ast {

parscore::source_location
target_def::start_loc() const
{
   if (explicit_tag_.valid())
      return explicit_tag_;
   else if (local_tag_.valid())
      return local_tag_;
   else
      return ri_->start_loc();
}

bool target_def::accept(visitor& v) const
{
   return v.visit(*this);
}

template<>
bool is_a<target_def>(const node& v) { return dynamic_cast<const target_def*>(&v); }

template<>
const target_def* as<target_def>(const node* v) { return dynamic_cast<const target_def*>(v); }

}}
