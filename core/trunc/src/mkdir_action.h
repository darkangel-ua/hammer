#if !defined(h_03d92fc0_2e9d_478b_a680_cc82af4504e2)
#define h_03d92fc0_2e9d_478b_a680_cc82af4504e2

#include "build_action.h"
#include "location.h"

namespace hammer
{
   class mkdir_action : public build_action
   {
      public:
         mkdir_action(const location_t& dir_to_create);
         virtual std::string target_tag(const build_node& node, const build_environment& environment) const;

      protected:
         virtual bool execute_impl(const build_node& node, const build_environment& environment) const;
      
      private:
         location_t dir_to_create_;
   };
}
#endif //h_03d92fc0_2e9d_478b_a680_cc82af4504e2
