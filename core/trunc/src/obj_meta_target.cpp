#include "stdafx.h"
#include <hammer/core/obj_meta_target.h>
#include <hammer/core/obj_main_target.h>
#include <hammer/core/types.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>

namespace hammer{

obj_meta_target::obj_meta_target(hammer::project* p, 
                                 const pstring& name, 
                                 const requirements_decl& req, 
                                 const requirements_decl& usage_req)
   : typed_meta_target(p, 
                       name, 
                       req, 
                       usage_req, 
                       p->engine()->get_type_registry().get(types::OBJ))
{
   set_explicit(true);
}

main_target* obj_meta_target::construct_main_target(const main_target* owner, const feature_set* properties) const
{
   return new obj_main_target(this, 
                              *owner,
                              name(), 
                              &type(), 
                              properties,
                              project()->engine()->targets_pool());

}

}
