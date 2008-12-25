#if !defined(h_51b99cdf_4b23_4067_b739_108519363f5b)
#define h_51b99cdf_4b23_4067_b739_108519363f5b

#include <string>
#include <vector>
#include "location.h"

namespace hammer
{
   class build_environment
   {
      public:
         virtual bool run_shell_commands(const std::vector<std::string>& cmds) const = 0;
         virtual const location_t& current_directory() const = 0;
         virtual void create_directories(const location_t& dir_to_create) const = 0;
         virtual void remove(const location_t& p) const = 0;
         virtual void copy(const location_t& source, const location_t& destination) const = 0;
         virtual ~build_environment() {}
   };
}

#endif //h_51b99cdf_4b23_4067_b739_108519363f5b
