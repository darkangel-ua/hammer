#include <iostream>
#include <iomanip>
#include <boost/filesystem/operations.hpp>
#include <hammer/core/engine.h>
#include <hammer/core/warehouse_manager.h>
#include <hammer/core/warehouse.h>
#include "build_cmd.h"
#include "package_cmd.h"

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

static
int publish_current_package(engine& e) {
   cout << "Publishing project at '" << fs::current_path() << "'... " << flush;

   if (!has_project_file(fs::current_path())) {
      cout << "Failed (File not found)" << endl;

      return 1;
   }

   const project& project_to_publish = e.load_project(fs::current_path());
   e.warehouse_manager().get_default()->add_to_packages(project_to_publish);

   cout << "Done\n" << endl;
   return 0;
}

static
int handle_publish_cmd(engine& e,
                       const string& project_query,
                       const unsigned debug_level) {
   if (!e.warehouse_manager().get_default()) {
      cout << "No warehouse found!" << endl;
      return 1;
   }

   if (project_query.empty())
      return publish_current_package(e);

   auto root = find_root(fs::current_path());
   if (!root) {
      cout << "Failed to find root project path!" << endl;
      return 1;
   }

   e.load_project(root.get());

   auto all_matched_projects = resolve_project_query(e, project_query);

   if (all_matched_projects.empty()) {
      cout << "Nothing to publish - query didn't match any of existing projects\n" << endl;
      return 0;
   }

   decltype(all_matched_projects) projects_to_publish;

   // filter out projects that already has been published into some warehouse
   for (auto& p : all_matched_projects) {
      if (!e.warehouse_manager().has_project(p.get().name(), p.get().publishable_version()))
         projects_to_publish.push_back(p);
   }

   if (projects_to_publish.empty()) {
      cout << "Nothing to publish - query matches some projects but all of them already has been published\n" << endl;
      return 0;
   }

   cout << "Projects to publish:\n" << endl;
   size_t max_project_name_length = 0;
   for (auto& p : projects_to_publish)
      max_project_name_length = std::max(max_project_name_length, p.get().name().length());

   for (auto& p : projects_to_publish) {
      cout << setw(max_project_name_length + 4) << left << (" /" + p.get().name()) << p.get().publishable_version() << endl;
   }

   cout << "\nPublish selected projects? [Y/n]: " << flush;
   char c;
   cin >> c;
   if (c != 'Y' && c != 'y')
      return 0;

   for (auto& p : projects_to_publish)
      e.warehouse_manager().get_default()->add_to_packages(p.get());

   return 0;
}

int handle_package_cmd(const std::vector<std::string>& args,
                       const unsigned debug_level) {
   auto engine = setup_engine(debug_level, false);

   if (args.size() == 1 && args.front() == "update") {
      if (!engine->warehouse_manager().get_default()) {
         cout << "No warehouse found!" << endl;
         return 1;
      }
      engine->warehouse_manager().get_default()->update();
   } else if (args.size() == 1 && args.front() == "refresh") {
      if (!engine->warehouse_manager().get_default()) {
         cout << "No warehouse found!" << endl;
         return 1;
      }
      engine->warehouse_manager().get_default()->update_all_packages(*engine);
   } else if (args.size() >= 1 && args.front() == "publish")
      return handle_publish_cmd(*engine, args.size() == 2 ? args[1] : string(), debug_level);
   else {
      show_package_cmd_help();
      return 1;
   }

   return 0;
}

void show_package_cmd_help() {
   cout << R"(usage: hammer package <command>

commands are:
   publish    publish current project in warehouse
   update     update packages info from warehouses
   refresh    reload packages that was modified in warehouses
)" << flush;

}
