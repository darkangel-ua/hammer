#if !defined(h_a21392dd_c752_4d63_bc95_b4432678152d)
#define h_a21392dd_c752_4d63_bc95_b4432678152d

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <string>
#include <memory>
#include "location.h"

namespace hammer
{
   class toolset;
   class engine;

   class toolset_manager
   {
      public:
         void add_toolset(std::auto_ptr<toolset>& t);
         void init_toolset(engine& e, 
                           const std::string& toolset_name, 
                           const std::string& toolset_version,
                           const location_t* toolset_home = NULL) const;

      private:
         typedef boost::unordered_map<std::string, boost::shared_ptr<toolset> > toolsets_t;

         toolsets_t toolsets_;
   };
}

#endif //h_a21392dd_c752_4d63_bc95_b4432678152d
