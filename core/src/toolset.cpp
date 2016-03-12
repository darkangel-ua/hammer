#include "stdafx.h"
#include <hammer/core/toolset.h>

namespace hammer{

toolset::toolset(const std::string& name) : name_(name) 
{}

void toolset::init(engine& e, const std::string& version_id, 
                   const location_t* toolset_home) const
{
   if (version_id.empty() && toolset_home != NULL)
      throw std::runtime_error("Toolset initialization error: toolset set install dir specified without version tag");
   
   init_impl(e, version_id, toolset_home);
}

}