#include "stdafx.h"
#include "cmdline_action.h"
#include "build_environment.h"

namespace hammer{

cmdline_action& cmdline_action::operator +=(const cmdline_builder& b)
{
   builders_.push_back(b);
   return *this;
}

void cmdline_action::execute_impl(const build_node& node, const build_environment& environment) const
{
   for(builders_t::const_iterator i = builders_.begin(), last = builders_.end(); i != last; ++i)
   {
      std::ostringstream s;
      i->write(s, node, environment);
      environment.run_shell_command(s.str());
   }
}

}
