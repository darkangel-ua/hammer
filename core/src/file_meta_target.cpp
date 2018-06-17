#include "stdafx.h"
#include <hammer/core/file_meta_target.h>
#include <hammer/core/file_main_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>

namespace hammer{

file_meta_target::file_meta_target(hammer::project* p, 
                                   const std::string& name,
                                   const std::string& filename,
                                   const requirements_decl& req, 
                                   const requirements_decl& usage_req)
   :
    meta_target(p, name, req, usage_req),
    filename_(filename)
{
}

main_target* 
file_meta_target::construct_main_target(const main_target* owner, 
                                        const feature_set* properties) const
{
   main_target* mt = new file_main_target(this, 
                                          name(), 
                                          filename_,
                                          properties,
                                          get_engine().get_type_registry().soft_resolve_from_target_name(filename_));
   return mt;
}

}
