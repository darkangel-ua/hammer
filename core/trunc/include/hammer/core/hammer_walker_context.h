#if !defined(h_61fc88e9_fff4_4c7a_a2b2_b69f7f92b327)
#define h_61fc88e9_fff4_4c7a_a2b2_b69f7f92b327

#include <boost/filesystem/path.hpp>

namespace hammer
{
   class engine;
   class project;
   class call_resolver;

   struct hammer_walker_context
   {
      hammer_walker_context() : project_(0) {}
      
      engine* engine_;
      call_resolver* call_resolver_;
      boost::filesystem::path location_;
      project* project_;
   };
}

#endif //h_61fc88e9_fff4_4c7a_a2b2_b69f7f92b327
