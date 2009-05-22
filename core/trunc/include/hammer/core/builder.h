#if !defined(h_a9fbc014_9e81_4066_929b_6e82414f8d81)
#define h_a9fbc014_9e81_4066_929b_6e82414f8d81

#include "build_environment.h"
#include "build_node.h"

namespace hammer
{
   class builder
   {
      public:
         typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;

         builder(const build_environment& environment, 
                 unsigned worker_count = 1, 
                 bool unconditional_build = false);
         ~builder();

         void build(nodes_t& nodes);
         void build(build_node& node);

      private:
         struct impl_t;
         impl_t* impl_;
   };
}

#endif //h_a9fbc014_9e81_4066_929b_6e82414f8d81
