#include <iostream>
#include <iomanip>
#include <yaml-cpp/emitter.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/toolset_manager.h>
#include <hammer/core/toolset.h>
#include "toolset_cmd.h"
#include "build_cmd.h"

using namespace std;
using namespace hammer;

static
void list_toolsets(engine& e,
                   std::ostream& os) {
   const toolset_manager& tm = e.toolset_manager();

   std::cout << "Configured toolsets are:\n" << std::endl;

   const bool empty_configuration = [&] {
      for (auto& t : tm)
         if (!t.second->empty())
            return false;
      return true;
   } ();

   if (empty_configuration) {
      std::cout << "   No toolsets were configured!\n" << std::endl;
      return;
   }

   YAML::Emitter em(os);
   em << YAML::BeginSeq;

   for (auto& t : tm) {
      if (t.second->empty())
         continue;

      em << YAML::BeginMap
         << YAML::Key
         << t.second->name();

      em << YAML::BeginSeq;
      bool first = true;
      for (auto& v : *t.second) {
         em << YAML::BeginMap
            << YAML::Key;

         if (first) {
            em << (v.version_ + " [default]");
            first = false;
         } else
            em << v.version_;

         em << YAML::Value << v.configuration_info_
            << YAML::EndMap;
      }
      em << YAML::EndSeq
         << YAML::EndMap;
   }

   em << YAML::EndSeq;
   std::cout << "\n" << std::endl;
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
