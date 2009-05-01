#include "stdafx.h"
#include <hammer/core/prebuilt_lib_meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>

namespace hammer{

prebuilt_lib_meta_target::prebuilt_lib_meta_target(hammer::project* p, 
                                                   const pstring& name, 
                                                   const pstring& lib_file_path,
                                                   const requirements_decl& props,
                                                   const requirements_decl& usage_req)
   :
    searched_lib_meta_target(p, 
                             name, 
                             lib_file_path, 
                             props, 
                             usage_req, 
                             p->get_engine()->get_type_registry().hard_resolve_from_target_name(lib_file_path))
{
}

}
