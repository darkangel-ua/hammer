#include <iostream>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <hammer/core/engine.h>
#include <hammer/core/build_environment_impl.h>
#include <hammer/core/testing_suite_meta_target.h>
#include <hammer/core/testing_meta_target.h>
#include <hammer/core/testing_compile_base_meta_target.h>
#include <hammer/core/testing_link_base_meta_target.h>
#include <hammer/core/basic_target.h>
#include "build_request.h"
#include "build_cmd.h"
#include "test_cmd.h"

using namespace std;
using namespace hammer;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace {

struct test_options_t {
   vector<string> test_request_;
   bool generate_report_;
} test_options;

po::options_description
test_options_description = []() {
   po::options_description desc("Test options");
   desc.add_options()
      ("report", po::bool_switch(&test_options.generate_report_), "Print test report to stdout")
   ;

   return desc;
}();

void parse_options(const vector<string>& args) {
   auto test_options_description_with_test_request = test_options_description;
   test_options_description_with_test_request.add_options()
      ("test-request", po::value<vector<string>>(&test_options.test_request_), "");

   po::positional_options_description test_request_description;
   test_request_description.add("test-request", -1);

   auto parsed_options =
      po::command_line_parser(args).
         options(test_options_description_with_test_request).
         positional(test_request_description).
         run();

   po::variables_map vm;
   po::store(parsed_options, vm);
   po::notify(vm);
}

bool is_testing_meta_target(const basic_meta_target& mt) {
   return dynamic_cast<const testing_suite_meta_target*>(&mt) ||
          dynamic_cast<const testing_meta_target*>(&mt) ||
          dynamic_cast<const testing_compile_base_meta_target*>(&mt) ||
          dynamic_cast<const testing_link_base_meta_target*>(&mt);
}

struct resolved_test_targets_t {
   std::vector<basic_target*> targets_;
   std::vector<std::string> unresolved_target_ids_;
};

resolved_test_targets_t
resolve_test_ids(const vector<string>& ids_to_resolve,
                 std::vector<basic_target*>& test_targets) {
   resolved_test_targets_t result;
   for (const string& id : ids_to_resolve) {
      auto i = find_if(test_targets.begin(), test_targets.end(), [=] (const basic_target* t) {
         return t->name() == id;
      });

      if (i != test_targets.end())
         result.targets_.push_back(*i);
      else
         result.unresolved_target_ids_.push_back(id);
   }

   return result;
}

vector<const basic_meta_target*>
gather_test_suites(const project& p) {
   vector<const basic_meta_target*> result;

   for (auto& t : p.targets()) {
      if (dynamic_cast<const testing_suite_meta_target*>(t.second.get()))
         result.push_back(t.second.get());
   }

   return result;
}

}


int handle_test_cmd(const std::vector<std::string>& args,
                    const unsigned debug_level,
                    volatile bool& interrupt_flag) {
   auto engine = setup_engine(debug_level);

   if (debug_level > 0)
      cout << "...Loading project at '" << fs::current_path() << "'... " << flush;

   const project* project_to_build = has_project_file(fs::current_path()) ? &engine->load_project(fs::current_path()) : nullptr;

   if (debug_level > 0)
      cout << static_cast<const char*>(project_to_build ? "Done" : "Done (Not Found)") << endl;

   if (!project_to_build) {
      if (debug_level > 0)
         cout << "...Searching root project ... " << flush;
      auto root = find_root(fs::current_path());
      if (!root) {
         if (debug_level > 0)
            cout << "Failed (Not Found)" << endl;
         return 1;
      } else {
         if (debug_level > 0)
            cout << "Found (at '" << root->string() << "')\n" << flush;
      }

      if (debug_level > 0)
         cout << "...Loading root project... " << endl;

      engine->load_project(*root);

      if (debug_level > 0)
         cout << "Done" << endl;
   }

   parse_options(args);

   auto build_request = resolve_build_request(*engine, test_options.test_request_, project_to_build);
   if (debug_level > 0)
      cout << build_request << flush;

   // here we resolve only top level targets
   auto resolved_targets = resolve_target_ids(*engine, project_to_build, build_request.target_ids_, *build_request.build_request_);

   if (debug_level > 0)
      cout << "Top level test targets are:\n"
           << resolved_targets
           << flush;

   for (auto& mt : resolved_targets.targets_) {
      if (!is_testing_meta_target(*mt)) {
         cout << "Failed: Target '" << mt->name() << "' at '" << mt->location() << "' is not a testing meta target!\n" << flush;
         return 1;
      }
   }

   cout << "...instantiating top level targets... " << flush;
   vector<basic_target*> instantiated_targets = instantiate(*engine, resolved_targets.targets_, *build_request.build_request_);
   cout << "Done" << endl;

   if (!resolved_targets.unresolved_target_ids_.empty() && project_to_build) {
      auto test_suites = gather_test_suites(*project_to_build);

      cout << "...instantiating test suites... " << flush;
      auto instantiated_suites = instantiate(*engine, test_suites, *build_request.build_request_);
      cout << "Done" << endl;

      auto test_targets = resolve_test_ids(resolved_targets.unresolved_target_ids_, instantiated_suites);
      if (!test_targets.unresolved_target_ids_.empty()) {
         cout << "Failed: unable to resolved passed target ids: " << boost::join(test_targets.unresolved_target_ids_, ", ") << endl;
         return 1;
      }

      if (debug_level > 0)
         cout << "\nTest suite targets are: " << boost::join(resolved_targets.unresolved_target_ids_, ", ") << "\n\n" << flush;

      resolved_targets.unresolved_target_ids_.clear();
      instantiated_targets.insert(instantiated_targets.end(), test_targets.targets_.begin(), test_targets.targets_.end());
   }

   if (!resolved_targets.unresolved_target_ids_.empty()) {
      cout << "Failed: Unable to resolve passed target ids: " << boost::join(resolved_targets.unresolved_target_ids_, ", ") << endl;
      return 1;
   }

   if (instantiated_targets.empty()) {
      cout << "Failed: Nothing to test!" << endl;
      return 1;
   }

   cout << "...generating build graph... " << flush;
   boost::optional<build_nodes_t> nodes = generate(*engine, instantiated_targets);
   if (!nodes) {
      cout << "Build failed\n";
      return 1;
   }

   cout << "Done" << endl;

   build(*engine, *nodes, debug_level, interrupt_flag);

   return 0;
}

void show_test_cmd_help() {
   cout << "usage: hammer test [options] <test request>\n\n"
        << test_options_description
        << flush;
}
