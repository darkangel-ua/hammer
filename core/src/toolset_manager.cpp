#include "stdafx.h"
#include <hammer/core/toolset_manager.h>
#include <hammer/core/toolset.h>
#include <hammer/core/feature_def.h>
#include <hammer/core/engine.h>

namespace hammer
{

void toolset_manager::add_toolset(std::auto_ptr<toolset> t)
{
   if (!toolsets_.insert(make_pair(t->name(), boost::shared_ptr<toolset>(t.get()))).second)
      throw std::runtime_error("Toolset '" + t->name() + "' already registered");
   t.release();
}

void toolset_manager::init_toolset(engine& e,
                                   const std::string& toolset_name,
                                   const std::string& toolset_version,
                                   const location_t* toolset_home) const
{
   toolsets_t::const_iterator i = toolsets_.find(toolset_name);
   if (i == toolsets_.end())
      throw std::runtime_error("Toolset '" + toolset_name + "' is not registered");

   i->second->init(e, toolset_version, toolset_home);
}

void toolset_manager::autoconfigure(engine& e) const
{
   for(toolsets_t::const_iterator i = toolsets_.begin(), last = toolsets_.end(); i != last; ++i)
      if (const feature_def* fd = e.feature_registry().find_def("toolset")) {
         if (!fd->is_legal_value(i->first))
            i->second->autoconfigure(e);
      }
}

}
