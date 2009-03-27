#include "stdafx.h"
#include <hammer/core/copy_target.h>
#include <cassert>
#include <hammer/core/copy_main_target.h>

namespace hammer{

copy_target::copy_target(const main_target* mt, const pstring& name,
                         const target_type* t, const feature_set* f)
   : file_target(mt, name, t, f)
{

}

const location_t& copy_target::location() const
{
   assert(dynamic_cast<const copy_main_target*>(get_main_target()));
   return static_cast<const copy_main_target*>(get_main_target())->destination();
}

}
