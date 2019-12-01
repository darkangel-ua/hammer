#pragma once

#include <iosfwd>
#include <hammer/core/build_node.h>

namespace hammer {

class project;

void generate_compile_database(std::ostream& os,
                               const project& p,
                               const build_nodes_t& nodes);

}

