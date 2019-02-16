#include <iostream>
#include <iomanip>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/program_options.hpp>
#include <hammer/core/engine.h>
#include <hammer/core/build_environment_impl.h>
#include <hammer/core/cleaner.h>
#include "build_cmd.h"
#include "build_request.h"
#include "clean_cmd.h"

using namespace std;
using namespace hammer;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace {

struct clean_options_t {
   // FIXME: only clean all supported
   bool clean_all_ = true;
   vector<string> clean_request_;
} clean_options;

po::options_description
clean_options_description = []() {
   po::options_description desc("Clean options");
   desc.add_options()
      ("clean-all", po::bool_switch(&clean_options.clean_all_), "clean all targets, including all dependencies")
   ;

   return desc;
}();

void parse_options(const vector<string>& args) {
   auto clean_options_description_with_clean_request = clean_options_description;
   clean_options_description_with_clean_request.add_options()
      ("clean-request", po::value<vector<string>>(&clean_options.clean_request_), "");

   po::positional_options_description clean_request_description;
   clean_request_description.add("clean-request", -1);

   auto parsed_options =
      po::command_line_parser(args).
         options(clean_options_description_with_clean_request).
         positional(clean_request_description).
         run();

   po::variables_map vm;
   po::store(parsed_options, vm);
   po::notify(vm);
}

}

int handle_clean_cmd(const std::vector<std::string>& args,
                     const unsigned debug_level) {
   auto engine = setup_engine(debug_level);

   if (debug_level > 0)
      cout << "...Loading project at '" << fs::current_path() << "'... " << flush;

   const project* project_to_clean = has_project_file(fs::current_path()) ? &engine->load_project(fs::current_path()) : nullptr;

   if (debug_level > 0)
      cout << static_cast<const char*>(project_to_clean ? "Done" : "Done (Not Founded)") << endl;

   parse_options(args);

   build_request clean_request = resolve_build_request(*engine, clean_options.clean_request_, project_to_clean);

   if (debug_level > 0) {
      cout << "\nClean request: " << dump_for_hash(*clean_request.build_request_)
           << "\nTargets to clean are:\n";
      for (auto target : clean_request.targets_)
         cout << "   " << target->name() << " at '" << target->location().string() << "'\n";
      cout << endl;
   }

   cout << "...instantiating... " << flush;
   vector<basic_target*> instantiated_targets = instantiate(*engine, clean_request.targets_, *clean_request.build_request_);
   cout << "Done" << endl;

   cout << "...generating graph... " << flush;
   boost::optional<build_nodes_t> nodes = generate(*engine, instantiated_targets);
   if (!nodes) {
      cout << "Failed" << endl;
      return 1;
   }
   cout << "Done" << endl;

   cout << "...cleaning..." << flush;
   build_environment_impl build_environment(fs::current_path());
   cleaner::result r = cleaner::clean_all(build_environment, *nodes);
   cout << "Done.\n"
        << r.cleaned_target_count_ << " targets was cleaned" << endl;

   return 0;
}

void show_clean_cmd_help() {
   cout << "usage: hammer clean [options] <clean request>\n\n"
        << clean_options_description
        << flush;
}
