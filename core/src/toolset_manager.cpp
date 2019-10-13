#include <hammer/core/toolset_manager.h>
#include <hammer/core/toolset.h>
#include <hammer/core/feature_def.h>
#include <hammer/core/engine.h>

namespace hammer {

void toolset_manager::add_toolset(engine& e,
                                  std::unique_ptr<toolset> t) {
   auto r = toolsets_.emplace(t->name(), std::move(t));
   if (!r.second)
      throw std::runtime_error("Toolset '" + t->name() + "' already registered");

   e.get_rule_manager().add(r.first->second->use_rule());
}

void toolset_manager::init_toolset(engine& e,
                                   const std::string& toolset_name,
                                   const std::string& toolset_version,
                                   const location_t* toolset_home) const {
   auto i = toolsets_.find(toolset_name);
   if (i == toolsets_.end())
      throw std::runtime_error("Toolset '" + toolset_name + "' is not registered");

   i->second->configure(e, toolset_version);
}

void toolset_manager::autoconfigure(engine& e) const {
   for (auto& toolset : toolsets_)
      if (const feature_def* fd = e.feature_registry().find_def("toolset")) {
         if (!fd->is_legal_value(toolset.first))
            toolset.second->autoconfigure(e);
      }
}

toolset_manager::~toolset_manager(){
}

}
