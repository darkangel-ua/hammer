#include <iostream>
#include <iomanip>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include "toolset_cmd.h"
#include "build_cmd.h"

using namespace std;
using namespace hammer;

static
void list_toolsets(engine& e,
                   std::ostream& os) {
   const feature_def* toolset_def = e.feature_registry().find_def("toolset");
   if (!toolset_def)
      return;

   for (auto& legal_value : toolset_def->legal_values()) {
      os << legal_value.value_;
      if (toolset_def->defaults_contains(legal_value.value_))
         os << " (default)\n";
      else
         os << "\n";

      const subfeature_def& toolset_version_def = toolset_def->get_subfeature("version");
      bool first = true;
      for (const string& v : toolset_version_def.legal_values(legal_value.value_)) {
         os << setw(4) << " " << v;
         if (first) {
            os << " (default)\n";
            first = false;
         } else
            os << "\n";
      }
   }
}

int handle_toolset_cmd(const std::vector<std::string>& args,
                       const unsigned debug_level) {
   if (args.size() != 1 || args.front() != "list") {
      show_toolset_cmd_help();
      return 1;
   }

   auto engine = setup_engine(debug_level);
   list_toolsets(*engine, cout);
   cout << flush;

   return 0;
}

void show_toolset_cmd_help() {
   cout << R"(usage: hammer toolset <command>

commands are:

   list    list all available toolsets

)" << flush;
}
