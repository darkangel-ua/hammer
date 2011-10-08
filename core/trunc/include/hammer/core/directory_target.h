#if !defined(h_5bf13480_428d_432a_8a52_559cfebef691)
#define h_5bf13480_428d_432a_8a52_559cfebef691

#include "basic_target.h"
#include "mkdir_action.h"

namespace hammer
{
   class directory_target : public basic_target
   {
      public:
         directory_target(const main_target* mt, const location_t& dir);
         const build_action* action() const { return &action_; }

      private:
         location_t dir_to_create_;
         mkdir_action action_;

         virtual std::vector<boost::intrusive_ptr<build_node> > generate() const;
         virtual void timestamp_info_impl() const;
   };
}

#endif //h_5bf13480_428d_432a_8a52_559cfebef691
