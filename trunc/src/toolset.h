#if !defined(h_7097ba14_7e00_4e62_88d3_9e89be1c6b41)
#define h_7097ba14_7e00_4e62_88d3_9e89be1c6b41

#include <string>
#include "location.h"

namespace hammer
{
   class engine;

   class toolset 
   {
      public:
         toolset(const std::string& name);

         const std::string name() const { return name_; }

         void init(engine& e, const std::string& version_id = std::string(), 
                   const location_t* toolset_home = NULL) const;

         virtual ~toolset() {}

      protected:
         virtual void init_impl(engine& e, const std::string& version_id = std::string(), 
                                const location_t* toolset_home = NULL) const = 0;

      private:
         std::string name_;
   };
}

#endif //h_7097ba14_7e00_4e62_88d3_9e89be1c6b41
