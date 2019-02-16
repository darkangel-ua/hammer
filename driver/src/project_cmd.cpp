#include <iostream>
#include <iomanip>
#include <cassert>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <hammer/core/engine.h>
#include <hammer/core/feature.h>
#include <hammer/core/subfeature.h>
#include <hammer/core/project_generators/msvc_solution.h>
#include "project_cmd.h"
#include "build_cmd.h"
#include "build_request.h"

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace {

struct project_options_t {
   vector<string> build_request_;
} project_options;

void parse_options(const vector<string>& args) {
   po::options_description desc("Project options");
   desc.add_options()
      ("build-request", po::value<vector<string>>(&project_options.build_request_), "");

   po::positional_options_description project_request_description;
   project_request_description.add("build-request", -1);

   auto parsed_options =
      po::command_line_parser(args).
         options(desc).
         positional(project_request_description).
         run();

   po::variables_map vm;
   po::store(parsed_options, vm);
   po::notify(vm);
}

}

int handle_project_cmd(const std::vector<std::string>& args,
                       const unsigned debug_level) {
   auto engine = setup_engine(debug_level);

   if (debug_level > 0)
      cout << "...Loading project at '" << fs::current_path() << "'... " << flush;

   const project* project_to_build = has_project_file(fs::current_path()) ? &engine->load_project(fs::current_path()) : nullptr;

   if (debug_level > 0)
      cout << static_cast<const char*>(project_to_build ? "Done" : "Failed (Not Founded)") << endl;

   if (!project_to_build)
      return 1;

   parse_options(args);

   build_request build_request = resolve_build_request(*engine, project_options.build_request_, project_to_build);

   if (debug_level > 0)
      cout << "...Build request: " << dump_for_hash(*build_request.build_request_) << endl;

   cout << "...instantiating... " << flush;
   vector<basic_target*> instantiated_targets = instantiate(*engine, build_request.targets_, *build_request.build_request_);
   cout << "Done" << endl;

   cout << "...generating build graph... " << flush;
   boost::optional<build_nodes_t> nodes = generate(*engine, instantiated_targets);
   if (!nodes) {
      cout << "Failed" << endl;
      return 1;
   }
   cout << "Done" << endl;

   auto i_toolset = build_request.build_request_->find("toolset");
   assert(i_toolset != build_request.build_request_->end());
   const auto toolset = (**i_toolset).value();
   const subfeature* toolset_version = (**i_toolset).find_subfeature("version");
   assert(toolset_version);
   if (toolset != "msvc" || toolset_version->value() != "8.0") {
      cout << "Error: Only msvc-8.0 toolset supports solution generation\n" << endl;
      return 1;
   }

   project_generators::msvc_solution solution(*project_to_build,
                                              project_to_build->location() / ".hammer",
                                              project_generators::msvc_solution::generation_mode::LOCAL);

   for (auto& node : *nodes)
      solution.add_target(node);

   solution.write();

   return 0;
}

void show_project_cmd_help() {
   cout << "usage: hammer project <build request>\n\n"
        << flush;
}
