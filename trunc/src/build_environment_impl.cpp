#include "stdafx.h"
#include "build_environment_impl.h"
#include <boost/filesystem/operations.hpp>
#include <boost/process.hpp>

namespace bp = boost::process;
using namespace std;

namespace hammer{

build_environment_impl::build_environment_impl(const location_t& cur_dir)
   : current_directory_(cur_dir)
{

}

void build_environment_impl::run_shell_commands(const std::vector<std::string>& cmds) const
{
   std::ostringstream s;
   if (!cmds.empty())
      s << '"' << cmds.front();
   
   if (cmds.size() > 1)
      for(vector<string>::const_iterator i = cmds.begin() + 1, last = cmds.end(); i != last; ++i)
         s << "&&" << *i;
   
   s << '"';

   bp::launcher launcher;
   launcher.set_stdin_behavior(bp::inherit_stream);
   launcher.set_stdout_behavior(bp::inherit_stream);
   launcher.set_stderr_behavior(bp::inherit_stream);
   launcher.start(bp::command_line::shell(s.str())).wait();
}

const location_t& build_environment_impl::current_directory() const
{
   return current_directory_;
}

void build_environment_impl::create_directories(const location_t& dir_to_create) const
{
   boost::filesystem::create_directories(dir_to_create);
}

}
