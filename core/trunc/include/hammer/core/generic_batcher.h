#if !defined(h_349dea65_202c_4f65_a838_baeec4cebfce)
#define h_349dea65_202c_4f65_a838_baeec4cebfce

#include <hammer/core/build_node.h>
#include <boost/unordered_set.hpp>

namespace hammer
{
   class generic_batcher
   {
      public:
         static void process(const build_node::nodes_t& nodes, unsigned concurency_level);
      
      private:
         typedef boost::unordered_set<const build_node*> visited_nodes_t;
         
         mutable visited_nodes_t visited_nodes_;
         unsigned concurency_level_;
         
         generic_batcher(unsigned concurency_level);
         void process_impl(build_node& node) const;
   };
}

#endif //h_349dea65_202c_4f65_a838_baeec4cebfce
