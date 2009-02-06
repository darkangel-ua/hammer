#include "stdafx.h"
#include <hammer/core/copy_meta_target.h>
#include <hammer/core/copy_main_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>

namespace hammer{

copy_meta_target::copy_meta_target(hammer::project* p, const pstring& name, 
                                   const requirements_decl& props,
                                   const requirements_decl& usage_req)
   : meta_target(p, name, props, usage_req)
{
}

main_target* copy_meta_target::construct_main_target(const feature_set* properties) const
{
   main_target* mt = new copy_main_target(this, 
                                          name(), 
                                          &project()->engine()->get_type_registry().get(types::COPIED), 
                                          properties,
                                          project()->engine()->targets_pool());
   return mt;
}

}
