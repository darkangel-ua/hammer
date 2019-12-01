#include <iostream>
#include <iomanip>
#include <cassert>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <hammer/core/engine.h>
#include <hammer/core/feature.h>
#include <hammer/core/subfeature.h>
#include <hammer/core/project_generators/msvc_solution.h>
#include <hammer/core/project_generators/compile_database.h>
#include "project_cmd.h"
#include "build_cmd.h"
#include "build_request.h"

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace {

struct project_options_t {
   std::string project_generator_;
   vector<string> build_request_;
} project_options;

void parse_options(const vector<string>& args) {
   po::options_description desc("Project options");
   desc.add_options()
      ("project-generator", po::value(&project_options.project_generator_), "")
      ("build-request", po::value<vector<string>>(&project_options.build_request_), "");

   po::positional_options_description project_request_description;
   project_request_description.add("project-generator", 1);
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

   if (project_options.project_generator_.empty() ||
       (project_options.project_generator_ != "msvc" &&
        project_options.project_generator_ != "compile-db"))
   {
       show_project_cmd_help();
       return -1;
   }

   auto build_request = resolve_build_request(*engine, project_options.build_request_, project_to_build);

   if (debug_level > 0)
      cout << build_request << endl;

   auto instantiator = [&] {
      auto resolved_targets = resolve_target_ids(*engine, project_to_build, build_request.target_ids_, *build_request.build_request_);

      cout << "...instantiating... " << flush;
      auto instantiated_targets = instantiate(*engine, resolved_targets.targets_, *build_request.build_request_);
      cout << "Done" << endl;
      return instantiated_targets;
   };

   boost::optional<build_nodes_t> nodes = generate(*engine, instantiator);
   if (!nodes) {
      cout << "Failed" << endl;
      return 1;
   }

   if (project_options.project_generator_ == "msvc") {
      auto i_toolset = build_request.build_request_->find("toolset");
      if ((**i_toolset).value() != "msvc") {
         std::cout << "Error: msvc generator must be used with msvc toolset\n";
         return -1;
      }

      project_generators::msvc_solution solution(*project_to_build,
                                                 project_to_build->location() / ".hammer",
                                                 project_generators::msvc_solution::generation_mode::LOCAL);

      for (auto& node : *nodes)
         solution.add_target(node);

      solution.write();
      return 0;
   }

   if (project_options.project_generator_ == "compile-db") {
      const fs::path compile_database_file = "compile_commands.json";
      fs::ofstream compile_database(compile_database_file);
      generate_compile_database(compile_database, *project_to_build, *nodes);
      compile_database.close();

      return 0;
   }

   return -1;
}

void show_project_cmd_help() {
   std::cout << "usage: hammer project <project generator> <build request>\n"
                "where <project generator> is one of:\n"
                "   msvc         Visual Studio solution\n"
                "   compile-db   compile_commands.json clang compilation database\n"
             << std::flush;
}
