#include <iostream>
#include <iomanip>
#include <hammer/core/engine.h>
#include <hammer/core/warehouse.h>
#include "warehouse_cmd.h"
#include "build_cmd.h"

using namespace hammer;
using namespace std;

int handle_warehouse_cmd(const std::vector<std::string>& args,
                         const unsigned debug_level) {
   auto engine = setup_engine(debug_level, false);

   if (args.size() != 1 || args.front() != "list") {
      show_warehouse_cmd_help();
      return 1;
   }

   cout << "Not implemented !!!" << endl;

   return 0;
}

void show_warehouse_cmd_help() {
   cout << R"(usage: hammer warehouse <command>

commands are:
   list     list configured warehouses

)" << flush;
}
