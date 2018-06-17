#include "stdafx.h"
#include <hammer/core/prebuilt_lib_meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/target_type.h>
#include <hammer/core/types.h>

namespace hammer{

static const target_type& resolve_type(const std::string& lib_file_path, const type_registry& tr)
{
   const target_type& lib_type = tr.hard_resolve_from_target_name(lib_file_path);
   if (lib_type.equal_or_derived_from(tr.get(types::SHARED_LIB)))
      return tr.get(types::PREBUILT_SHARED_LIB);
   else 
      return tr.get(types::PREBUILT_STATIC_LIB);
}

prebuilt_lib_meta_target::prebuilt_lib_meta_target(hammer::project* p, 
                                                   const std::string& name,
                                                   const std::string& lib_file_path,
                                                   const requirements_decl& props,
                                                   const requirements_decl& usage_req)
   :
    searched_lib_meta_target(p, 
                             name, 
                             lib_file_path, 
                             props, 
                             usage_req, 
                             resolve_type(lib_file_path, p->get_engine().get_type_registry()))
{
}

}
