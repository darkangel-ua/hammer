#include <iostream>
#include <iomanip>
#include <hammer/core/engine.h>
#include <hammer/core/warehouse.h>
#include <hammer/core/warehouse_manager.h>
#include "warehouse_cmd.h"
#include "build_cmd.h"

using namespace hammer;
using namespace std;

struct warehouse_desc {
   string id_;
   boost::filesystem::path storage_dir_;
};

int handle_warehouse_cmd(const std::vector<std::string>& args,
                         const unsigned debug_level) {
   auto engine = setup_engine(debug_level, false);

   if (args.size() != 1 || args.front() != "list") {
      show_warehouse_cmd_help();
      return 1;
   }

   warehouse_manager& whm = engine->warehouse_manager();
   vector<warehouse_desc> whl;
   transform(whm.begin(), whm.end(), back_inserter(whl), [](const warehouse_manager::value_type& wh) {
      return warehouse_desc{ wh.second->id_, wh.second->storage_dir_ };
   });

   sort(whl.begin(), whl.end(), [](const warehouse_desc& rhs, const warehouse_desc& lhs) {
      return rhs.id_ < lhs.id_;
   });

   unsigned id_len = 0;
   for (const auto& i : whl)
      id_len = std::max(id_len, (unsigned)i.id_.size());

   cout << "Configured warehouses are:\n" << endl;
   for (const auto& i : whl) {
      cout << setw(id_len + 3) << left << i.id_ << i.storage_dir_.string() << endl;
   }

   cout << endl;

   return 0;
}

void show_warehouse_cmd_help() {
   cout << R"(usage: hammer warehouse <command>

commands are:
   list     list configured warehouses

)" << flush;
}
