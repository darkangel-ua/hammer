#include "stdafx.h"
#include <hammer/core/output_dir_argument_writer.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/build_node.h>
#include <hammer/core/location.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/build_environment.h>

namespace hammer{

void output_dir_argument_writer::write_impl(std::ostream& output, 
                                            const build_node& node, 
                                            const build_environment& environment) const
{
   if (node.products_.empty())
      return;

//   location_t output_dir = relative_path(node.products_.front()->mtarget()->intermediate_dir(), environment.current_directory());
   location_t output_dir = relative_path(node.products_.front()->get_main_target()->intermediate_dir(), node.products_.front()->get_main_target()->location());
   output_dir.normalize();
   output << output_dir.string();
}

}

