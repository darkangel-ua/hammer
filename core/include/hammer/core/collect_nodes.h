#if !defined(h_6e1d50f7_f656_4b13_ae22_ede555873b94)
#define h_6e1d50f7_f656_4b13_ae22_ede555873b94

#include <vector>
#include <set>
#include "build_node.h"

namespace hammer
{
   class target_type;

   void collect_nodes(build_node::sources_t& result, 
                      std::set<const build_node*>& visited_nodes,
                      const std::vector<boost::intrusive_ptr<build_node> >& sources, 
                      const std::vector<const target_type*>& types_to_collect,
                      bool recursive);
}

#endif //h_6e1d50f7_f656_4b13_ae22_ede555873b94
