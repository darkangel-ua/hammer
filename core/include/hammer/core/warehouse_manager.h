#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <hammer/core/location.h>

namespace hammer {

class warehouse;
class project;

class warehouse_manager : public boost::noncopyable {
      using warehouses_t = std::unordered_map<std::string, std::unique_ptr<warehouse>>;

   public:
      using const_iterator = warehouses_t::const_iterator;
      using value_type = warehouses_t::value_type;

      warehouse_manager();
      void insert(std::unique_ptr<warehouse> wh);
      const_iterator begin() const { return warehouses_.begin(); }
      const_iterator end() const { return warehouses_.end(); }
      const_iterator find(const std::string& id) const;
      const_iterator find(const project& p) const;

      warehouse* get_default() const { return default_; }
      ~warehouse_manager();

   private:
      warehouse* default_ = nullptr;
      warehouses_t warehouses_;
};

}
