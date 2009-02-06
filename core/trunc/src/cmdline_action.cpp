#include "stdafx.h"
#include "cmdline_action.h"
#include "build_environment.h"
#include <iostream>
#include <fstream>

using namespace std;

namespace hammer{

cmdline_action& cmdline_action::operator +=(const cmdline_builder& b)
{
   builders_.push_back(b);
   return *this;
}

bool cmdline_action::execute_impl(const build_node& node, const build_environment& environment) const
{
   if (rsp_builder_.get() != NULL)
   {
      string rsp_file_name(target_tag(node, environment) + ".rsp");
      auto_ptr<ostream> rsp_stream(environment.create_output_file(rsp_file_name.c_str(), ios_base::trunc));
      rsp_builder_->write(*rsp_stream, node, environment);
   }

   std::vector<std::string> commands;
   for(builders_t::const_iterator i = builders_.begin(), last = builders_.end(); i != last; ++i)
   {
      std::ostringstream s;
      i->write(s, node, environment);
      commands.push_back(s.str());
   }

   return run_shell_commands(commands, node, environment);
}

bool cmdline_action::run_shell_commands(const std::vector<std::string>& commands, 
                                        const build_node& node, 
                                        const build_environment& environment) const
{
   return environment.run_shell_commands(commands);
}

std::string cmdline_action::target_tag(const build_node& node, const build_environment& environment) const
{
   std::ostringstream s;
   target_writer_->write(s, node, environment);
   return s.str();
}

}
