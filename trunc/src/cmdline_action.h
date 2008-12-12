#if !defined(h_a1e58555_b259_43a7_a0f6_9c225178390e)
#define h_a1e58555_b259_43a7_a0f6_9c225178390e

#include "build_action.h"
#include "cmdline_builder.h"
#include <vector>
#include "cmdline_builder.h"

namespace hammer
{
   class cmdline_action : public build_action
   {
      public:
         cmdline_action() {}
         cmdline_action& operator +=(const cmdline_builder& b);

      protected:
         virtual void execute_impl(const build_node& node, const build_environment& environment) const;

      private:
         typedef std::vector<cmdline_builder> builders_t;

         builders_t builders_;
   };
}

#endif //h_a1e58555_b259_43a7_a0f6_9c225178390e
