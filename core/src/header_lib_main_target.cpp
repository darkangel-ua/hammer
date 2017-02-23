#include "stdafx.h"
#include <hammer/core/header_lib_main_target.h>

namespace hammer{
header_lib_main_target::header_lib_main_target(const basic_meta_target* mt,
                                               const std::string& name,
                                               const target_type* t, 
                                               const feature_set* props)
   : main_target(mt, name, t, props)
{

}

}
