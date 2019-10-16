#include "stdafx.h"
#include <cstdio>
#include <fstream>
#include <iterator>
#include <iostream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/regex.hpp>
#include <boost/process.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/thread/thread.hpp>
#include <hammer/core/build_environment_impl.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/main_target.h>
#include <hammer/core/basic_build_target.h>

namespace bp = boost::process;
namespace fs = boost::filesystem;
using namespace std;

namespace hammer{

build_environment_impl::build_environment_impl(const location_t& cur_dir,
                                               const bool print_shell_commands)
   : current_directory_(cur_dir),
     print_shell_commands_(print_shell_commands),
     cache_directory_(cur_dir / ".hammer/bin"),
     shell_executor_guard_(shell_executor_.get_executor())
{
   shell_executor_thread_ = std::thread([&] {
      shell_executor_.run();
   });
}

build_environment_impl::~build_environment_impl() {
   shell_executor_guard_.reset();
   shell_executor_thread_.join();
}

bool build_environment_impl::run_shell_commands(std::ostream* captured_output_stream,
                                                std::ostream* captured_error_stream,
                                                const std::vector<std::string>& cmds,
                                                const location_t& working_dir) const {
   auto tmp_file_name = to_string(boost::uuids::random_generator{}()) + ".cmd";
   location_t full_tmp_file_name(working_dir / tmp_file_name);
   full_tmp_file_name.normalize();
   const std::string fullcmd = boost::join(cmds, "\n");

   try
   {
#if defined(_WIN32)
      {
         std::unique_ptr<ostream> f(create_output_file(full_tmp_file_name.string().c_str(), std::ios_base::out));
         *f << fullcmd;
      }
#endif

#if defined(_WIN32)
      auto shell_cmd = bp::search_path("cmd.exe");
      const std::vector<std::string> cmd_args = {"/Q", "/C", "call", tmp_file_name};

      if (print_shell_commands_ && captured_output_stream)
         dump_shell_command(*captured_output_stream, full_tmp_file_name);
#else
      auto shell_cmd = bp::shell();
      const std::vector<std::string> cmd_args;

      if (print_shell_commands_ && captured_output_stream)
         *captured_output_stream << fullcmd;
#endif

      boost::system::error_code system_ec;
      int system_exit_code = 0;

      std::promise<void> shell_promise;
      auto shell_barrier = shell_promise.get_future();

      int running_coroutines = 0;
      std::array<char, 1024> outbuf;
      std::array<char, 1024> errbuf;

      auto on_co_end = [&] {
         if (++running_coroutines == 3)
            shell_promise.set_value();
      };

      shell_executor().post([&] {
         auto output_pipe = std::make_shared<bp::async_pipe>(shell_executor());
         auto error_pipe = std::make_shared<bp::async_pipe>(shell_executor());

         bp::async_system(shell_executor(),
                          [&](const boost::system::error_code& ec, int exit_code) { system_ec = ec; system_exit_code = exit_code; on_co_end(); },
                          shell_cmd, bp::args = cmd_args,
                          bp::std_in = boost::asio::buffer(fullcmd),
                          bp::std_out > *output_pipe,
                          bp::std_err > *error_pipe,
                          bp::start_dir(working_dir));

         // TODO: this need to be converted to coroutines
         auto output_thread = std::make_shared<std::function<void()>>();
         *output_thread = [&, output_thread, output_pipe] {
            output_pipe->async_read_some(boost::asio::buffer(outbuf), [&] (const boost::system::error_code& ec, std::size_t transferred) {
               if (transferred) {
                  if (captured_output_stream) {
                     captured_output_stream->write(outbuf.data(), transferred);
                     captured_output_stream->flush();
                  } else {
                     std::cout.write(outbuf.data(), transferred);
                     std::cout.flush();
                  }
               }

               if (ec)
                  return on_co_end();

               (*output_thread)();
            });
         };

         // TODO: this need to be converted to coroutines
         auto error_thread = std::make_shared<std::function<void()>>();
         *error_thread = [&, error_thread, error_pipe] {
            error_pipe->async_read_some(boost::asio::buffer(errbuf), [&] (const boost::system::error_code& ec, std::size_t transferred) {
               if (transferred) {
                  if (!captured_error_stream) {
                     std::cerr.write(errbuf.data(), transferred);
                     std::cerr.flush();
                  }

                  if (captured_error_stream) {
                     captured_error_stream->write(errbuf.data(), transferred);
                     captured_error_stream->flush();
                  } else {
                     std::cout.write(errbuf.data(), transferred);
                     std::cout.flush();
                  }
               }

               if (ec)
                  return on_co_end();

               (*error_thread)();
            });
         };

         (*output_thread)();
         (*error_thread)();
      });

      // waiting for shell command complete
      shell_barrier.get();

      const bool success = (system_exit_code == 0 && !system_ec);
      if (!success)
#if defined(_WIN32)
         dump_shell_command(captured_error_stream ? *captured_error_stream : cerr, full_tmp_file_name);
#else
         (captured_error_stream ? *captured_error_stream : cerr) << fullcmd;
#endif


#if defined(_WIN32)
      remove(full_tmp_file_name);
#endif

      return success;
   } catch(const std::exception& e) {
      (captured_error_stream ? *captured_error_stream : cerr) << "Error: " << e.what() << std::endl;
   }

#if defined(_WIN32)
   dump_shell_command(captured_error_stream ? *captured_error_stream : cerr, full_tmp_file_name);
   remove(full_tmp_file_name);
#else
   (captured_error_stream ? *captured_error_stream : cerr) << fullcmd;
#endif
   return false;
}

static
std::unique_ptr<std::istream>
open_input_stream(const location_t& full_content_file_name) {
#if defined(_WIN32) && !defined(__MINGW32__)
	wstring unc_path(L"\\\\?\\" + to_wide(location_t(full_content_file_name)).string<wstring>());
   std::unique_ptr<ifstream> f(new ifstream);
	f->open(unc_path.c_str());
   return f;
#else
   std::unique_ptr<istream> f(new ifstream((full_content_file_name).string().c_str()));
   return f;
#endif
}

void build_environment_impl::dump_shell_command(std::ostream& s,
                                                const location_t& full_content_file_name) const {
   std::unique_ptr<istream> f(open_input_stream(full_content_file_name));
   s << '\n';
   std::copy(istreambuf_iterator<char>(*f), istreambuf_iterator<char>(), ostreambuf_iterator<char>(s));
   s << '\n';
}

const location_t&
build_environment_impl::current_directory() const {
   return current_directory_;
}

void build_environment_impl::create_directories(const location_t& dir_to_create) const {
   if (dir_to_create.has_root_path())
      boost::filesystem::create_directories(dir_to_create);
   else
      boost::filesystem::create_directories(current_directory_ / dir_to_create);
}

void build_environment_impl::remove(const location_t& p) const {
   boost::filesystem::remove(p);
}

void build_environment_impl::remove_file_by_pattern(const location_t& dir,
                                                    const std::string& pattern) const {
   boost::regex rpattern(pattern);
   using iter = fs::directory_iterator;
   for (iter i = iter(dir), last = iter(); i != last; ++i) {
      fs::file_status st = i->status();
      if (!is_directory(st) && boost::regex_match(i->path().filename().string(), rpattern))
         remove(i->path());
   }
}

void build_environment_impl::copy(const location_t& full_source_path,
                                  const location_t& full_destination_path) const {
   boost::filesystem::copy_file(full_source_path, full_destination_path);
}

bool build_environment_impl::write_tag_file(const std::string& filename,
                                            const std::string& content) const {
   ofstream f(filename.c_str(), ios_base::trunc);
   if (!f)
      return false;

   f << content;
   f.close();

   return true;
}

std::unique_ptr<ostream>
build_environment_impl::create_output_file(const char* filename,
                                           ios_base::openmode mode) const {
   location_t full_filename_path(filename);
   if (!full_filename_path.has_root_path())
      full_filename_path = current_directory() / full_filename_path;

   full_filename_path.normalize();
   unique_ptr<ofstream> f(new ofstream);
   f->exceptions(ios_base::badbit | ios_base::eofbit | ios_base::failbit);
#if defined(_WIN32) && !defined(__MINGW32__)
   wstring unc_path(to_wide(location_t(full_filename_path)).string<wstring>());
   f->open(unc_path.c_str(), mode);
#else
   f->open(full_filename_path.string().c_str(), mode);
#endif
   // FIXME: msvc 12 can't just return f
   return std::unique_ptr<ostream>(std::move(f));
}

location_t
build_environment_impl::working_directory(const basic_build_target& t) const {
   return t.get_main_target()->location();
}

const location_t*
build_environment_impl::cache_directory() const {
   return &cache_directory_;
}

std::ostream&
build_environment_impl::output_stream() const {
   return std::cout;
}

ostream&
build_environment_impl::error_stream() const {
   return std::cerr;
}

}
