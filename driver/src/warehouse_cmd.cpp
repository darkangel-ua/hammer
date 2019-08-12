#include <iostream>
#include <yaml-cpp/emitter.h>
#include <hammer/core/engine.h>
#include <hammer/core/warehouse.h>
#include <hammer/core/warehouse_manager.h>
#include "warehouse_cmd.h"
#include "build_cmd.h"

using namespace hammer;

namespace {

struct winfo {
   const std::string& id_;
   YAML::Node info_;
};

}

int handle_warehouse_cmd(const std::vector<std::string>& args,
                         const unsigned debug_level) {
   auto engine = setup_engine(debug_level, false);

   if (args.size() != 1 || args.front() != "list") {
      show_warehouse_cmd_help();
      return 1;
   }

   std::cout << "Configured warehouses are:\n" << std::endl;

   warehouse_manager& whm = engine->warehouse_manager();
   if (whm.size() == 0) {
      std::cout << "   No warehouses were configured!\n" << std::endl;
      return 0;
   }

   // FIXME: YAML::Node cannot handle simple std::swap, so we doing indirect sorts
   // and use std::list just in case
   std::list<winfo> infos;
   for (const auto& wh : whm)
      infos.push_back({wh.second->id_, wh.second->info()});

   infos.sort([] (const winfo& lhs,
                  const winfo& rhs) {
      return lhs.id_ < rhs.id_;
   });

   YAML::Emitter em(std::cout);
   em << YAML::BeginSeq;
      for (const auto& info : infos) {
         em << YAML::BeginMap
            << YAML::Key;

         if (info.id_ == whm.get_default()->id_)
            em << (info.id_ + " [default]");
         else
            em << info.id_;

         em << YAML::Value << info.info_
            << YAML::EndMap;
      }
   em << YAML::EndSeq;
   std::cout << "\n" << std::endl;

   return 0;
}

void show_warehouse_cmd_help() {
   std::cout << R"(usage: hammer warehouse <command>

commands are:
   list     list configured warehouses

)" << std::flush;
}
