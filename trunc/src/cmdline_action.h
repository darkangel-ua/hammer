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
         
         template<typename T>
         cmdline_action(const std::string& name, 
                        boost::shared_ptr<T>& target_writer)
         : build_action(name),
           target_writer_(shared_dynamic_cast<argument_writer>(target_writer))
         {
         }

         virtual std::string target_tag(const build_node& node, const build_environment& environment) const;

         cmdline_action& operator +=(const cmdline_builder& b);

      protected:
         virtual bool execute_impl(const build_node& node, const build_environment& environment) const;

      private:
         typedef std::vector<cmdline_builder> builders_t;

         builders_t builders_;
         boost::shared_ptr<argument_writer> target_writer_;
   };
}

#endif //h_a1e58555_b259_43a7_a0f6_9c225178390e
