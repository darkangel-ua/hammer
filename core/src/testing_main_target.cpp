#include "stdafx.h"
#include <cassert>
#include <hammer/core/testing_main_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/output_location_strategy.h>

namespace hammer{

testing_main_target::testing_main_target(const meta_target* mt, 
                                         const pstring& name, 
                                         const target_type* t, 
                                         const feature_set* props,
                                         pool& p)
   : main_target(mt, name, t, props, p)
{}

location_t testing_main_target::intermediate_dir_impl() const
{
   assert(!sources().empty());
   return get_engine()->output_location_strategy().compute_output_location(dynamic_cast<const main_target&>(*sources().front()));
}

}
