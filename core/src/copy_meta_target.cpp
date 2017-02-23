#include "stdafx.h"
#include <hammer/core/copy_meta_target.h>
#include <hammer/core/copy_main_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>

namespace hammer{

copy_meta_target::copy_meta_target(hammer::project* p, const std::string& name,
                                   const requirements_decl& props,
                                   const requirements_decl& usage_req)
   : meta_target(p, name, props, usage_req)
{
}

main_target* copy_meta_target::construct_main_target(const main_target* owner, const feature_set* properties) const
{
   main_target* mt = new copy_main_target(this, 
                                          name(), 
                                          &get_engine()->get_type_registry().get(types::COPIED), 
                                          properties);
   return mt;
}

}
