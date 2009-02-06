#if !defined(h_6439ef3d_edef_4bf6_a284_fde25f91214a)
#define h_6439ef3d_edef_4bf6_a284_fde25f91214a

#include "cmdline_action.h"

namespace hammer
{
   class testing_run_action : public cmdline_action
   {
      public:
         template<typename T>
         testing_run_action(const std::string& name, 
                            boost::shared_ptr<T>& target_writer,
                            boost::shared_ptr<T>& output_writer)
         : cmdline_action(name, target_writer),
           output_writer_(shared_static_cast<argument_writer>(output_writer))
         {
         }
      
      protected:
         virtual bool run_shell_commands(const std::vector<std::string>& commands,
                                         const build_node& node, 
                                         const build_environment& environment) const;

      private:
         boost::shared_ptr<argument_writer> output_writer_;
   };
}

#endif //h_6439ef3d_edef_4bf6_a284_fde25f91214a
