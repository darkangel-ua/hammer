#include "stdafx.h"
#include <hammer/core/header_lib_main_target.h>

namespace hammer{
header_lib_main_target::header_lib_main_target(const hammer::meta_target* mt, 
                                               const pstring& name, 
                                               const target_type* t, 
                                               const feature_set* props,
                                               pool& p)
   : main_target(mt, name, t, props, p)
{

}

}
