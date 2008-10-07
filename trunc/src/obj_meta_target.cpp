#include "stdafx.h"
#include "obj_meta_target.h"
#include "obj_main_target.h"
#include "project.h"
#include "engine.h"

namespace hammer{

// FIXME: in instantiate we must ensure that only simply targets is in sources.
obj_meta_target::obj_meta_target(hammer::project* p, const pstring& name, 
                                 const requirements_decl& req, 
                                 const requirements_decl& usage_req)
                                :
                                 meta_target(p, name, req, usage_req)
{

}

main_target* obj_meta_target::construct_main_target(const feature_set* properties) const
{
   main_target* mt = new(project()->engine()->targets_pool()) 
                        obj_main_target(this, 
                                        name(), 
                                        properties,
                                        project()->engine()->targets_pool());
   return mt;
}

}