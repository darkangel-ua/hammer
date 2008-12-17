#include "stdafx.h"
#include "mkdir_action.h"
#include "build_environment.h"
#include "fs_helpers.h"
#include "build_node.h"
#include "basic_target.h"
#include "main_target.h"
#include "meta_target.h"

namespace hammer{

mkdir_action::mkdir_action(const location_t& dir_to_create)
   : dir_to_create_(dir_to_create)
{

}

void mkdir_action::execute_impl(const build_node& node, const build_environment& environment) const
{
   environment.create_directories(relative_path(dir_to_create_, node.products_.front()->mtarget()->meta_target()->location()));
}

}
