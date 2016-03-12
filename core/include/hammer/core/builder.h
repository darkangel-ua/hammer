#if !defined(h_a9fbc014_9e81_4066_929b_6e82414f8d81)
#define h_a9fbc014_9e81_4066_929b_6e82414f8d81

#include "build_environment.h"
#include "build_node.h"
#include <iosfwd>

namespace hammer
{
   class project;
   class builder
   {
      public:
         typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;
         
         struct result
         {
            result() : updated_targets_(0),
                       failed_to_build_targets_(0),
                       skipped_targets_(0) 
            {}

            unsigned updated_targets_;
            unsigned failed_to_build_targets_;
            unsigned skipped_targets_;
         };

         builder(const build_environment& environment, 
                 volatile bool& interrupt_flag,
                 unsigned worker_count, 
                 bool unconditional_build);
         ~builder();

         result build(nodes_t& nodes, const project* bounds = NULL);
         result build(build_node& node, const project* bounds = NULL);
         void generate_graphviz(std::ostream& os, nodes_t& nodes, const project* bounds = NULL);

      private:
         struct impl_t;
         impl_t* impl_;
   };
}

#endif //h_a9fbc014_9e81_4066_929b_6e82414f8d81
