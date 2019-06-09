#include "stdafx.h"
#include <hammer/core/batched_cmdline_action.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/batch_tag_writer.h>
#include <hammer/core/build_node.h>
#include <hammer/core/main_target.h>

using namespace std;

namespace hammer{

batched_cmdline_action::batched_cmdline_action(const std::string& name)
   : cmdline_action(name, std::shared_ptr<batch_tag_writer>(new batch_tag_writer))
{

}

bool batched_cmdline_action::execute_impl(const build_node& node,
                                          const build_environment& environment) const
{
   return cmdline_action::execute_impl(node, environment);
}

}
