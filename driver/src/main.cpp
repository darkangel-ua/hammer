#include "stdafx.h"
#include <unordered_set>
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_set.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include <signal.h>

#include <hammer/core/engine.h>
#include <hammer/core/feature.h>
#include <hammer/core/subfeature.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project_generators/msvc_solution.h>
#include <hammer/core/testing_generators.h>
#include <hammer/core/obj_generator.h>
#include <hammer/core/build_environment_impl.h>
#include <hammer/core/builder.h>
#include <hammer/core/cleaner.h>
#include <hammer/core/actuality_checker.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/header_lib_generator.h>
#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/toolsets/gcc_toolset.h>
#include <hammer/core/toolsets/qt_toolset.h>
#include <hammer/core/htmpl/htmpl.h>
#include <hammer/core/toolset_manager.h>
#include <hammer/core/scaner_manager.h>
#include <hammer/core/c_scanner.h>
#include <hammer/core/generic_batcher.h>
#include <hammer/core/collect_nodes.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/warehouse_target.h>
#include <hammer/core/warehouse.h>
#include <hammer/core/generated_build_target.h>

#include "version.h"

#include "build_cmd.h"
#include "clean_cmd.h"
#include "toolset_cmd.h"
#include "project_cmd.h"
#include "warehouse_cmd.h"
#include "package_cmd.h"
#include "test_cmd.h"

using namespace std;
using namespace hammer;
using namespace hammer::project_generators;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

typedef vector<boost::intrusive_ptr<build_node> > nodes_t;

namespace
{
   // Ctrl-C flag
   volatile bool interrupt_flag = false;

   // Reaction in Ctrl-C
   volatile bool terminate_immediately = true;

   void already_interrupted_ctrl_handler(int sig)
   {
      signal(SIGINT, already_interrupted_ctrl_handler);
   }

   void ctrl_handler(int sig)
   {
      signal(SIGINT, already_interrupted_ctrl_handler);
      interrupt_flag = true;
      printf("...interrupting...\n");

      if (terminate_immediately)
         terminate();
   }

   using raw_args = vector<string>;

   struct global_options {
      bool version_ = false;
      bool help_ = false;
      unsigned debug_level_ = 0;
      string command_;
      raw_args command_args_;
   } global_options;

   po::options_description
   global_options_description = []() {
      po::options_description desc{"Global options"};
      desc.add_options()
         ("help", po::bool_switch(&global_options.help_), "produce this help message")
         ("version", po::bool_switch(&global_options.version_), "print version information")
         ("debug-level,d", po::value<unsigned>(&global_options.debug_level_), "debug level")
         ("command", po::value<string>(&global_options.command_), "")
         ("command-args", po::value<raw_args>(&global_options.command_args_), "")
         ;
      return desc;
   } ();

void print_global_help() {
   cout << R"(usage: hammer [--version] [--help]
              <command> [args]

commands are:
   build      Run build process
   test       Build and run tests
   clean      Run clean process
   package    Manage packages
   toolset    Manage toolsets
   project    Generate IDE project
   warehouse  Manage warehouses

build command is default and invoked when no other command specified.

)";
}

}

static
std::unordered_set<string> top_commands =
   { "build", "test", "clean", "toolset", "project", "warehouse", "package" };

int main(int argc, char** argv) {
   try {
      po::options_description desc{global_options_description};
      po::variables_map vm;

      po::positional_options_description pos;
      pos.add("command", 1).
          add("command-args", -1);

      po::parsed_options parsed_options = po::command_line_parser(argc, argv).
         options(desc).
         positional(pos).
         allow_unregistered().
         run();

      po::store(parsed_options, vm);
      po::notify(vm);

      global_options.command_args_ = [&] () {
         // we collect all options including positional 'command' and then remove it from beginning
         // this way we get rest of cmdline in right order (i.e. not modified)
         auto opts = po::collect_unrecognized(parsed_options.options, po::include_positional);
         if (!opts.empty() &&
             !global_options.command_.empty() &&
             top_commands.find(global_options.command_) != top_commands.end())
         {
            opts.erase(opts.begin());
         }

         return opts;
      } ();

      const string& cmd = global_options.command_;

      if (global_options.help_) {
         if ( cmd.empty() )
            print_global_help();
         else if (cmd == "build")
            show_build_cmd_help();
         else if (cmd == "test")
            show_test_cmd_help();
         else if (cmd == "clean")
            show_clean_cmd_help();
         else if (cmd == "toolset")
            show_toolset_cmd_help();
         else if (cmd == "project")
            show_project_cmd_help();
         else if (cmd == "warehouse")
            show_warehouse_cmd_help();
         else if (cmd == "package")
            show_package_cmd_help(global_options.command_args_);
         else {
            cout << "Unknown command: " << cmd << endl;
            return 1;
         }

         return 0;
      }

      if (global_options.version_) {
         print_version(cout);
         cout << endl << endl;

         return 0;
      }

      if (cmd == "clean")
         return handle_clean_cmd(global_options.command_args_, global_options.debug_level_);
      else if (cmd == "test") {
         signal(SIGINT, ctrl_handler);
         terminate_immediately = false;
         return handle_test_cmd(global_options.command_args_, global_options.debug_level_, interrupt_flag);
      } else if (cmd == "toolset")
         return handle_toolset_cmd(global_options.command_args_, global_options.debug_level_);
      else if (cmd == "project")
         return handle_project_cmd(global_options.command_args_, global_options.debug_level_);
      else if (cmd == "warehouse")
         return handle_warehouse_cmd(global_options.command_args_, global_options.debug_level_);
      else if (cmd == "package")
         return handle_package_cmd(global_options.command_args_, global_options.debug_level_);
      else {
         signal(SIGINT, ctrl_handler);
         terminate_immediately = false;
         return handle_build_cmd(global_options.command_args_, global_options.debug_level_, interrupt_flag);
      }
   } catch (const hammer::parsing_error& e) {
      cout << e.what() << "\n";
      return -1;
   } catch (const std::exception& e) {
      cout << "Error: " << e.what() << "\n";
      return -1;
   } catch (...) {
      cout << "Error: Unknown error.\n";
      return -1;
   }
}
