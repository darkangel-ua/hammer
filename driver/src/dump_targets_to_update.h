#pragma once
#include <iosfwd>
#include <hammer/core/build_node.h>
#include <hammer/core/build_environment.h>

void dump_targets_to_update(std::ostream& os, 
                            const hammer::build_node::nodes_t& nodes,
                            hammer::build_environment& environment);
