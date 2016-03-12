#include "stdafx.h"
#include <hammer/core/obj_main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/output_location_strategy.h>

namespace hammer{

obj_main_target::obj_main_target(const hammer::meta_target* mt, 
                                 const main_target& owner,
                                 const pstring& name, 
                                 const target_type* t, 
                                 const feature_set* props,
                                 pool& p)
                                : 
                                 main_target(mt, name, t, props, p),
                                 owner_(owner)
{
}

location_t obj_main_target::intermediate_dir_impl() const
{
   return get_engine()->output_location_strategy().compute_output_location(owner_);
}

}

