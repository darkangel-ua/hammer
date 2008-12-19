#include "stdafx.h"
#include "cmdline_action.h"
#include "build_environment.h"
#include <iostream>

namespace hammer{

cmdline_action& cmdline_action::operator +=(const cmdline_builder& b)
{
   builders_.push_back(b);
   return *this;
}

bool cmdline_action::execute_impl(const build_node& node, const build_environment& environment) const
{
   std::vector<std::string> commands;
   for(builders_t::const_iterator i = builders_.begin(), last = builders_.end(); i != last; ++i)
   {
      std::ostringstream s;
      i->write(s, node, environment);
      commands.push_back(s.str());
   }

   return environment.run_shell_commands(commands);
}

std::string cmdline_action::target_tag(const build_node& node, const build_environment& environment) const
{
   std::ostringstream s;
   target_writer_->write(s, node, environment);
   return s.str();
}

}
