#if !defined(h_aadb12b2_e4c3_4e98_8b8b_f253679476f9)
#define h_aadb12b2_e4c3_4e98_8b8b_f253679476f9

#include <hammer/core/cmdline_action.h>

namespace hammer
{
   class batched_cmdline_action : public cmdline_action
   {
      public:
         batched_cmdline_action(const std::string& name);

      protected:
         virtual bool execute_impl(const build_node& node, const build_environment& environment) const;
   };
}

#endif //h_aadb12b2_e4c3_4e98_8b8b_f253679476f9
