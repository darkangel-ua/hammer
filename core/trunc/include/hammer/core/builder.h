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
                 unsigned worker_count = 1, 
                 bool unconditional_build = false);
         ~builder();

         result build(nodes_t& nodes);
         result build(build_node& node);

      private:
         struct impl_t;
         impl_t* impl_;
   };
}

#endif //h_a9fbc014_9e81_4066_929b_6e82414f8d81
