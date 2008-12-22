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
      ofstream f(rsp_file_name.c_str(), ios_base::trunc);
      if (!f)
         throw runtime_error("Can't open response file '" + rsp_file_name + "' for writing");

      rsp_builder_->write(f, node, environment);
      
      if (!f)
         throw std::runtime_error("Error while writing to response file '" + rsp_file_name + "'");
      
      f.close();
   }

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
