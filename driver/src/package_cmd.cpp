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
int handle_publish_cmd(engine& e,
                       const unsigned debug_level) {
   if (!e.warehouse_manager().get_default()) {
      cout << "No warehouse found!" << endl;
      return 1;
   }

   if (debug_level > 0)
      cout << "...Loading project at '" << fs::current_path() << "'... " << flush;

   if (!has_project_file(fs::current_path())) {
      cout << "Failed (Not Found)" << endl;
      return 1;
   }

   const project& project_to_publish = e.load_project(fs::current_path());
   e.warehouse_manager().get_default()->add_to_packages(project_to_publish);

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
   } else if (args.size() == 1 && args.front() == "publish")
      return handle_publish_cmd(*engine, debug_level);
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
