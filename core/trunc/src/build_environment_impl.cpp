#include "stdafx.h"
#include <fstream>
#include <iterator>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/regex.hpp>
#include <boost/process.hpp>
#include <boost/guid.hpp>
#include <hammer/core/build_environment_impl.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/main_target.h>

namespace bp = boost::process;
namespace fs = boost::filesystem;
using namespace std;

namespace hammer{

build_environment_impl::build_environment_impl(const location_t& cur_dir)
   : current_directory_(cur_dir), cache_directory_(cur_dir / ".hammer/bin")
{

}

bool build_environment_impl::run_shell_commands(std::ostream* captured_output_stream,
                                                const std::vector<std::string>& cmds,
                                                const location_t& working_dir) const
{
   string tmp_file_name(boost::guid::create().to_string() + ".cmd");
   location_t full_tmp_file_name(working_dir / tmp_file_name);
   full_tmp_file_name.normalize();
   std::stringstream cmd_stream;

   try
   {
#if defined(_WIN32)
      {
         std::auto_ptr<ostream> f(create_output_file(full_tmp_file_name.native_file_string().c_str(), std::ios_base::out));

         for(vector<string>::const_iterator i = cmds.begin(), last = cmds.end(); i != last; ++i)
            *f << *i << '\n';
      }
#else
      for(vector<string>::const_iterator i = cmds.begin(), last = cmds.end(); i != last; ++i)
         cmd_stream << *i << '\n';
#endif

      bp::context ctx;
      ctx.environment = bp::self::get_environment();
      ctx.stdin_behavior = bp::close_stream();
      ctx.work_directory = working_dir.native_file_string();

      if (captured_output_stream != NULL)
      {
         ctx.stdout_behavior = bp::capture_stream();
         ctx.stderr_behavior = bp::redirect_stream_to_stdout();
      }
      else
      {
         ctx.stdout_behavior = bp::inherit_stream();
         ctx.stderr_behavior = bp::inherit_stream();
      }
      
     std::vector<std::string> cmdline;
#if defined(_WIN32)
      cmdline.push_back("cmd.exe");
      cmdline.push_back("/Q");
      cmdline.push_back("/C");
      cmdline.push_back("call " + tmp_file_name);
#endif

#if defined(_WIN32)
      bp::child shell_action_child = bp::launch(std::string(), cmdline, ctx);
#else
      bp::child shell_action_child = bp::launch_shell(cmd_stream.str(), ctx);
#endif

      if (captured_output_stream != NULL)
         std::copy(istreambuf_iterator<char>(shell_action_child.get_stdout()),
                   istreambuf_iterator<char>(),
                   ostreambuf_iterator<char>(*captured_output_stream));

      bp::status st = shell_action_child.wait();

      if (st.exit_status() != 0)
#if defined(_WIN32)
         dump_shell_command(std::cerr, full_tmp_file_name);
#else
         cerr << cmd_stream.str();
#endif


#if defined(_WIN32)
      remove(full_tmp_file_name);
#endif

      return st.exit_status() == 0;
   }
   catch(const std::exception& e)
   {
      std::cerr << "Error: " << e.what() << std::endl;
   }
   catch(...)
   {
      std::cerr << "Error: Unknown error\n";
   }

#if defined(_WIN32)
   dump_shell_command(std::cerr, full_tmp_file_name);
   remove(full_tmp_file_name);
#else
   cerr << cmd_stream.str();
#endif
   return false;
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
   std::stringstream s;
   bool result = run_shell_commands(&s, cmds, working_dir);
   captured_output = s.str();
   return result;
}

bool build_environment_impl::run_shell_commands(std::ostream& captured_output_stream, 
                                                const std::vector<std::string>& cmds, 
                                                const location_t& working_dir) const
{
   return run_shell_commands(&captured_output_stream, cmds, working_dir);
}

static std::auto_ptr<std::istream> open_input_stream(const location_t& full_content_file_name)
{
#if defined(_WIN32) && !defined(__MINGW32__)
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
#if defined(_WIN32) && !defined(__MINGW32__)
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

std::ostream& build_environment_impl::output_stream() const
{
   return std::cout;
}

}
