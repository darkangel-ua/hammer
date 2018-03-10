#if !defined(h_a21392dd_c752_4d63_bc95_b4432678152d)
#define h_a21392dd_c752_4d63_bc95_b4432678152d

#include <unordered_map>
#include <string>
#include <memory>
#include "location.h"
#include <hammer/core/toolset.h>

namespace hammer
{
   class engine;

   class toolset_manager
   {
      public:
         // add toolset to manager and register toolset::use_rule() in rule_manager
			void add_toolset(engine& e,
			                 std::unique_ptr<toolset> t);
         void init_toolset(engine& e, 
                           const std::string& toolset_name, 
                           const std::string& toolset_version,
                           const location_t* toolset_home = NULL) const;
         void autoconfigure(engine& e) const;
         ~toolset_manager();

      private:
         typedef std::unordered_map<std::string, std::unique_ptr<toolset>> toolsets_t;

         toolsets_t toolsets_;
   };
}

#endif //h_a21392dd_c752_4d63_bc95_b4432678152d
