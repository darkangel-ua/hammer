#include "stdafx.h"
#include "build_environment_impl.h"
#include <boost/filesystem/operations.hpp>
#include <boost/process.hpp>
#include <boost/guid.hpp>
#include <fstream>
#include <iterator>
namespace bp = boost::process;
using namespace std;

namespace hammer{

build_environment_impl::build_environment_impl(const location_t& cur_dir)
   : current_directory_(cur_dir)
{

}

bool build_environment_impl::run_shell_commands(const std::vector<std::string>& cmds) const
{
   string tmp_file_name(boost::guid::create().to_string() + ".cmd");
   try
   {
      ofstream f(tmp_file_name.c_str());

      for(vector<string>::const_iterator i = cmds.begin(), last = cmds.end(); i != last; ++i)
         f << *i << '\n';
      f.close();

      bp::launcher launcher;
      launcher.set_stdin_behavior(bp::inherit_stream);
      launcher.set_stdout_behavior(bp::inherit_stream);
      launcher.set_stderr_behavior(bp::inherit_stream);
      bp::command_line cmdline = bp::command_line("cmd.exe");
      cmdline.argument("/Q").argument("/C").argument("call " + tmp_file_name);
      bp::child shell_action_child = launcher.start(cmdline);
      bp::status st = shell_action_child.wait();

      if (st.exit_status() != 0)
      {
         dump_shell_command(std::cerr, tmp_file_name);
      }

      boost::filesystem::remove(tmp_file_name);
      
      return st.exit_status() == 0;
   }
   catch(...)
   {
      dump_shell_command(std::cerr, tmp_file_name);
      boost::filesystem::remove(tmp_file_name);
      return false;
   }
}

void build_environment_impl::dump_shell_command(std::ostream& s, const std::string& content_file_name) const
{
   ifstream f(content_file_name.c_str());
   s << '\n';
   copy(istreambuf_iterator<char>(f), istreambuf_iterator<char>(), ostreambuf_iterator<char>(s));
   s << '\n';
}

const location_t& build_environment_impl::current_directory() const
{
   return current_directory_;
}

void build_environment_impl::create_directories(const location_t& dir_to_create) const
{
   boost::filesystem::create_directories(dir_to_create);
}

void build_environment_impl::remove(const location_t& p) const
{
   boost::filesystem::remove(p);
}

}
