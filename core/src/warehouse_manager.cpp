#include <cassert>
#include <hammer/core/warehouse.h>
#include <hammer/core/warehouse_manager.h>

namespace hammer {

warehouse_manager::warehouse_manager() = default;

void warehouse_manager::insert(std::unique_ptr<warehouse> wh) {
   assert(find(wh->id_) == end());
   if (!default_)
      default_ = wh.get();
   warehouses_.insert({wh->id_, std::move(wh)});
}

warehouse_manager::~warehouse_manager() {

}

warehouse_manager::const_iterator
warehouse_manager::find(const std::string& warehouse_id) const {
   return warehouses_.find(warehouse_id);
}

warehouse_manager::const_iterator
warehouse_manager::find(const project& p) const {
   return std::find_if(warehouses_.begin(), warehouses_.end(), [&](const value_type& wh) {
      return wh.second->owned(p);
   });
}

bool warehouse_manager::has_project(const location_t& project_id,
                                    const std::string& version) const {
   return std::find_if(warehouses_.begin(), warehouses_.end(), [&](const value_type& wh) {
      return wh.second->has_project(project_id, version);
   }) != warehouses_.end();
}

}
