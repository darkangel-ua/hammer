#include "stdafx.h"
#include <hammer/core/obj_meta_target.h>
#include <hammer/core/types.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>

namespace hammer{

obj_meta_target::obj_meta_target(hammer::project* p, 
                                 const std::string& name,
                                 const requirements_decl& req, 
                                 const requirements_decl& usage_req)
   : typed_meta_target(p, 
                       name, 
                       req, 
                       usage_req, 
                       p->get_engine().get_type_registry().get(types::OBJ))
{
}

}
