#include "stdafx.h"
#include <hammer/core/default_output_location_strategy.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <iostream>

using namespace std;

namespace hammer{

location_t default_output_location_strategy::compute_output_location(const main_target& mt) const
{
   return mt.get_project()->location() / ".hammer/bin" / mt.name().to_string() / mt.hash_string();
}

}
