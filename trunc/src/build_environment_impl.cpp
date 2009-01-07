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

bool build_environment_impl::run_shell_commands(std::string* captured_output, const std::vector<std::string>& cmds) const
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
      
      if (captured_output != NULL)
      {
         launcher.set_stdout_behavior(bp::redirect_stream);
         launcher.set_stderr_behavior(bp::close_stream);
         launcher.set_merge_out_err(true);
      }
      else
      {
         launcher.set_stdout_behavior(bp::inherit_stream);
         launcher.set_stderr_behavior(bp::inherit_stream);
      }

      bp::command_line cmdline = bp::command_line("cmd.exe");
      cmdline.argument("/Q").argument("/C").argument("call " + tmp_file_name);
      bp::child shell_action_child = launcher.start(cmdline);
      
      if (captured_output != NULL)
         std::copy(istreambuf_iterator<char>(shell_action_child.get_stdout()), 
                   istreambuf_iterator<char>(), 
                   back_inserter(*captured_output));

      bp::status st = shell_action_child.wait();

      if (st.exit_status() != 0)
         dump_shell_command(std::cerr, tmp_file_name);

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

bool build_environment_impl::run_shell_commands(const std::vector<std::string>& cmds) const
{
   return run_shell_commands(NULL, cmds);
}

bool build_environment_impl::run_shell_commands(std::string& captured_output, const std::vector<std::string>& cmds) const
{
   return run_shell_commands(&captured_output, cmds);
}

void build_environment_impl::dump_shell_command(std::ostream& s, const std::string& content_file_name) const
{
   ifstream f(content_file_name.c_str());
   s << '\n';
   std::copy(istreambuf_iterator<char>(f), istreambuf_iterator<char>(), ostreambuf_iterator<char>(s));
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

void build_environment_impl::copy(const location_t& source, const location_t& destination) const
{
   boost::filesystem::copy_file(source, destination);
}

bool build_environment_impl::write_tag_file(const std::string& filename, const std::string& content) const
{
   ofstream f(filename.c_str(), ios_base::trunc);
   if (!f)
      return false;

   f << content;
   f.close();

   return true;
}

std::auto_ptr<ostream> build_environment_impl::create_output_file(const char* filename, ios_base::_Openmode mode) const
{
   std::auto_ptr<ofstream> f(new ofstream);
   f->exceptions(ios_base::badbit | ios_base::eofbit | ios_base::failbit);
   f->open(filename, mode);

   return f;
}

}
