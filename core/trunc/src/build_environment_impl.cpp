#include "stdafx.h"
#include <hammer/core/build_environment_impl.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/main_target.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>
#include <boost/process.hpp>
#include <boost/guid.hpp>
#include <fstream>
#include <iterator>

namespace bp = boost::process;
namespace fs = boost::filesystem;
using namespace std;

namespace hammer{

build_environment_impl::build_environment_impl(const location_t& cur_dir)
   : current_directory_(cur_dir), cache_directory_(cur_dir / ".hammer/bin")
{

}

bool build_environment_impl::run_shell_commands(std::string* captured_output, 
                                                const std::vector<std::string>& cmds, 
                                                const location_t& working_dir) const
{
   string tmp_file_name(boost::guid::create().to_string() + ".cmd");
   location_t full_tmp_file_name(working_dir / tmp_file_name);

   try
   {
      {
         std::auto_ptr<ostream> f(create_output_file(full_tmp_file_name.native_file_string().c_str(), std::ios_base::out));

         for(vector<string>::const_iterator i = cmds.begin(), last = cmds.end(); i != last; ++i)
            *f << *i << '\n';
      }

      bp::launcher launcher;
      launcher.set_stdin_behavior(bp::inherit_stream);
      launcher.set_work_directory(working_dir.native_file_string());

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

#if defined(_WIN32)
      bp::command_line cmdline = bp::command_line("cmd.exe");
      cmdline.argument("/Q").argument("/C").argument("call " + tmp_file_name);
#else
      const char* shell_cmd = getenv("SHELL");
      if (shell_cmd == NULL)
         throw std::runtime_error("Can't find SHELL environment variable.");
      bp::command_line cmdline = bp::command_line(shell_cmd);
      cmdline.argument(tmp_file_name);
#endif      
      bp::child shell_action_child = launcher.start(cmdline);

      
      if (captured_output != NULL)
         std::copy(istreambuf_iterator<char>(shell_action_child.get_stdout()), 
                   istreambuf_iterator<char>(), 
                   back_inserter(*captured_output));

      bp::status st = shell_action_child.wait();

      if (st.exit_status() != 0)
         dump_shell_command(std::cerr, full_tmp_file_name);

      remove(full_tmp_file_name);
      
      return st.exit_status() == 0;
   }
   catch(...)
   {
      dump_shell_command(std::cerr, full_tmp_file_name);
      remove(full_tmp_file_name);
      return false;
   }
}

bool build_environment_impl::run_shell_commands(const std::vector<std::string>& cmds, 
                                                const location_t& working_dir) const
{
   return run_shell_commands(NULL, cmds, working_dir);
}

bool build_environment_impl::run_shell_commands(std::string& captured_output, 
                                                const std::vector<std::string>& cmds, 
                                                const location_t& working_dir) const
{
   return run_shell_commands(&captured_output, cmds, working_dir);
}

static std::auto_ptr<std::istream> open_input_stream(const location_t& full_content_file_name)
{
#if defined(_WIN32)
   std::auto_ptr<istream> f(new ifstream((L"\\\\?\\" + to_wide(location_t(full_content_file_name)).native_file_string()).c_str()));
   return f;
#else
   std::auto_ptr<istream> f(new ifstream((full_content_file_name).native_file_string().c_str()));
   return f;
#endif
}

void build_environment_impl::dump_shell_command(std::ostream& s, const location_t& full_content_file_name) const
{
   std::auto_ptr<istream> f(open_input_stream(full_content_file_name));
   s << '\n';
   std::copy(istreambuf_iterator<char>(*f), istreambuf_iterator<char>(), ostreambuf_iterator<char>(s));
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

void build_environment_impl::remove_file_by_pattern(const location_t& dir, const std::string& pattern) const
{
   boost::regex rpattern(pattern);
   typedef fs::directory_iterator iter;
   for(iter i = iter(dir), last = iter(); i != last; ++i)
   {
      fs::file_status st = i->status();
      if (!is_directory(st) && boost::regex_match(i->path().filename(), rpattern))
         remove(i->path());
   }
}

void build_environment_impl::copy(const location_t& full_source_path, const location_t& full_destination_path) const
{
   boost::filesystem::copy_file(full_source_path, full_destination_path);
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

std::auto_ptr<ostream> build_environment_impl::create_output_file(const char* filename, ios_base::openmode mode) const
{
   location_t full_filename_path(filename);
   if (!full_filename_path.has_root_path())
      full_filename_path = current_directory() / full_filename_path;

   full_filename_path.normalize();
   std::auto_ptr<ofstream> f(new ofstream);
   f->exceptions(ios_base::badbit | ios_base::eofbit | ios_base::failbit);
#if defined (_WIN32)
   wstring unc_path(L"\\\\?\\" + to_wide(location_t(full_filename_path)).native_file_string());
   f->open(unc_path.c_str(), mode);
#else
   f->open(full_filename_path.native_file_string().c_str(), mode);
#endif
   return auto_ptr<ostream>(f.release());
}

location_t build_environment_impl::working_directory(const basic_target& t) const
{
   return t.get_main_target()->location();
}

const location_t* build_environment_impl::cache_directory() const
{
   return &cache_directory_;
}

}
