#pragma once
#include <vector>
#include <set>
#include <hammer/core/build_node.h>

namespace hammer {

class target_type;

void collect_nodes(build_node::sources_t& result,
                   std::set<const build_node*>& visited_nodes,
                   const std::vector<boost::intrusive_ptr<build_node> >& sources,
                   const std::vector<const target_type*>& types_to_collect,
                   bool recursive);
}
