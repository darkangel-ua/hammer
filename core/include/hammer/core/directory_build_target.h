#if !defined(h_5bf13480_428d_432a_8a52_559cfebef691)
#define h_5bf13480_428d_432a_8a52_559cfebef691

#include <hammer/core/generated_build_target.h>
#include <hammer/core/mkdir_action.h>

namespace hammer
{
   class directory_build_target : public generated_build_target
   {
      public:
         directory_build_target(const main_target* mt,
			                       const location_t& dir);
         const build_action* action() const { return &action_; }
			const location_t& location() const override { return dir_to_create_; }
			void clean(const build_environment& environment) const override;

      private:
         location_t dir_to_create_;
         mkdir_action action_;

         void timestamp_info_impl() const override;
   };
}

#endif //h_5bf13480_428d_432a_8a52_559cfebef691
