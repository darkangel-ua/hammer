#pragma once
#include <map>
#include <string>
#include <memory>
#include <hammer/core/location.h>

namespace hammer {

class engine;
class toolset;

class toolset_manager {
      using name_t = std::string;
      typedef std::map<name_t, std::unique_ptr<toolset>> toolsets_t;

   public:
      using const_iterator = toolsets_t::const_iterator;

      // add toolset to manager and register toolset::use_rule() in rule_manager
      void add_toolset(engine& e,
                       std::unique_ptr<toolset> t);
      void init_toolset(engine& e,
                        const std::string& toolset_name,
                        const std::string& toolset_version,
                        const location_t* toolset_home = NULL) const;
      void autoconfigure(engine& e) const;
      ~toolset_manager();

      const_iterator begin() const { return toolsets_.begin(); }
      const_iterator end() const { return toolsets_.end(); }
      bool empty() const { return toolsets_.empty(); }

   private:
      toolsets_t toolsets_;
};

}
