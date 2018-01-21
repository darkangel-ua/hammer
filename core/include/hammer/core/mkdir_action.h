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
         std::string target_tag(const build_node& node,
			                       const build_environment& environment) const override;
			std::vector<const feature*> valuable_features() const override { return {}; }

      protected:
         bool execute_impl(const build_node& node,
			                  const build_environment& environment) const override;
      
      private:
         location_t dir_to_create_;
   };
}
#endif //h_03d92fc0_2e9d_478b_a680_cc82af4504e2
