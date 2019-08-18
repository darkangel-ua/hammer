#include <iostream>
#include <iomanip>
#include <functional>
#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>
#include <hammer/core/engine.h>
#include <hammer/core/warehouse_manager.h>
#include <hammer/core/warehouse.h>
#include "build_cmd.h"
#include "package_cmd.h"

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace {

int publish_current_package(engine& e,
                            warehouse& target_warehouse) {
   cout << "Publishing project at '" << fs::current_path().string() << "' ... " << flush;

   if (!has_project_file(fs::current_path())) {
      cout << "Failed (File not found)" << endl;

      return 1;
   }

   const project& project_to_publish = e.load_project(fs::current_path());
   target_warehouse.add_to_packages(project_to_publish);

   cout << "Done\n" << endl;

   return 0;
}

int handle_publish_cmd(engine& e,
                       warehouse& target_warehouse,
                       const string& project_query,
                       const unsigned debug_level) {
   if (project_query.empty())
      return publish_current_package(e, target_warehouse);

   auto root = find_root(fs::current_path());
   if (!root) {
      cout << "Failed to find hamroot!" << endl;
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
      if (!target_warehouse.has_project(p.get().name(), p.get().publishable_version()))
         projects_to_publish.push_back(p);
   }

   if (projects_to_publish.empty()) {
      cout << "Nothing to publish - query matches some projects, but all of them already has been published\n" << endl;
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

   cout << endl;

   for (auto& p : projects_to_publish) {
      cout << "Publishing project at '" << p.get().location().string() << "' into warehouse '" << target_warehouse.id_ << "' ... " << flush;
      target_warehouse.add_to_packages(p.get());
      cout << "Done" << endl;
   }

   cout << endl;

   return 0;
}

struct {
   std::string warehouse_id_;
} warehouse_options;

po::options_description
warehouse_options_description = [] () {
   po::options_description desc{"Valid options"};
   desc.add_options()
      ("warehouse,w", po::value<string>(&warehouse_options.warehouse_id_), "Warehouse id to operate on")
      ;
   return desc;
} ();

void parse_options(vector<string> args) {
   po::variables_map vm;

   po::parsed_options parsed_options = po::command_line_parser{args}.
      options(warehouse_options_description).
      run();

   po::store(parsed_options, vm);
   po::notify(vm);
}

warehouse&
get_warehouse(engine& e,
              const std::string& warehouse_id) {
   if (warehouse_id.empty()) {
      auto default_warehouse = e.warehouse_manager().get_default();
      if (!default_warehouse)
         throw std::runtime_error("Default warehouse has not been configured!");

      return *default_warehouse;
   } else {
      auto i = e.warehouse_manager().find(warehouse_id);
      if (i == e.warehouse_manager().end())
         throw std::runtime_error("Warehouse '" + warehouse_id + "' not found!");
      return *i->second;
   }
}

std::vector<std::reference_wrapper<warehouse>>
get_warehouses(engine& e,
               const std::string& warehouse_id) {
   if (warehouse_id.empty()) {
      std::vector<std::reference_wrapper<warehouse>> result;
      for (auto& wp : e.warehouse_manager())
         result.push_back(*wp.second);

      return result;
   } else {
      auto i = e.warehouse_manager().find(warehouse_id);
      if (i == e.warehouse_manager().end())
         throw std::runtime_error("Warehouse '" + warehouse_id + "' not found!");

      return {1, *i->second};
   }
}

void update(engine& e,
            std::ostream& os) {
   auto warehouses = get_warehouses(e, warehouse_options.warehouse_id_);
   if (warehouses.empty()) {
      os << "Nothing to update! Looks like no warehouses were configured\n";
      return;
   }

   os << "Updating warehouses:\n" << std::endl;

   for (warehouse& w : warehouses) {
      os << w.id_ << " ... " << std::flush;
      try {
         w.update();
         os << "Done" << std::endl;
      } catch (const std::exception& e) {
         os << "Failed: " << e.what() << std::endl;
      }
   }

   os << std::endl;
}

}

int handle_package_cmd(std::vector<std::string> args,
                       const unsigned debug_level) {
   if (args.empty()) {
      cout << "missing command!\n";
      show_package_cmd_help({});
      return 1;
   }

   auto engine = setup_engine(debug_level, false);

   const auto cmd = args.front();
   args.erase(args.begin());

   if (cmd == "update") {
      parse_options(args);
      update(*engine, std::cout);
   } else if (cmd == "refresh") {
      parse_options(args);
      get_warehouse(*engine, warehouse_options.warehouse_id_).update_all_packages(*engine);
   } else if (cmd == "publish") {
      string project_query;
      po::options_description desc{warehouse_options_description};
      desc.add_options()
         ("query", po::value<string>(&project_query), "")
         ;
      po::positional_options_description pos;
      pos.add("query", 1);

      po::parsed_options parsed_options = po::command_line_parser(args).
         options(desc).
         positional(pos).
         run();

      po::variables_map vm;
      po::store(parsed_options, vm);
      po::notify(vm);

      return handle_publish_cmd(*engine, get_warehouse(*engine, warehouse_options.warehouse_id_), project_query, debug_level);
   } else {
      cout << "Unknown command: " << args.front() << endl;
      show_package_cmd_help({});
      return 1;
   }

   return 0;
}

void show_package_cmd_help(const std::vector<std::string>& args) {
   if (!args.empty()) {
      const auto cmd = args.front();
      if (cmd == "update") {
         cout << "usage: hammer package update [<options>]\n\n" << warehouse_options_description << endl;
         return;
      } else if (cmd == "refresh") {
         cout << "usage: hammer package refresh [<options>]\n\n" << warehouse_options_description << endl;
         return;
      } else if (cmd == "publish") {
         cout << "usage: hammer package publish <project query> [<options>]\n\n" << warehouse_options_description << endl;
         return;
      }
   }

   cout << R"(usage: hammer package <command>

commands are:
   publish    publish current project to warehouse
   update     update packages info from warehouses
   refresh    reload packages that was modified in warehouses
)" << flush;

}
