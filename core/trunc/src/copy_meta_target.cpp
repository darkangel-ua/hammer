#include "stdafx.h"
#include "copy_meta_target.h"
#include "copy_main_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "types.h"

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
