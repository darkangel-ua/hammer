#if !defined(h_51b99cdf_4b23_4067_b739_108519363f5b)
#define h_51b99cdf_4b23_4067_b739_108519363f5b

#include <string>
#include "location.h"

namespace hammer
{
   class build_environment
   {
      public:
         virtual void run_shell_command(const std::string& cmd) const = 0;
         virtual const location_t& current_directory() const = 0;
         virtual ~build_environment() {}
   };
}

#endif //h_51b99cdf_4b23_4067_b739_108519363f5b
