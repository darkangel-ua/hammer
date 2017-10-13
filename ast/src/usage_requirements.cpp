#include <hammer/ast/usage_requirements.h>
#include <hammer/ast/visitor.h>
#include <hammer/ast/casts.h>

namespace hammer{ namespace ast{

usage_requirements::usage_requirements(const expression* requirements)
   : requirements_(requirements)
{}

parscore::source_location
usage_requirements::start_loc() const
{
   return requirements_->start_loc();
}

bool usage_requirements::accept(visitor& v) const
{
   return v.visit(*this);
}

template<>
bool is_a<usage_requirements>(const node& v) { return dynamic_cast<const usage_requirements*>(&v); }

}}

