#if !defined(h_612363a8_96e9_4e1f_8fdd_706368d197d9)
#define h_612363a8_96e9_4e1f_8fdd_706368d197d9

#include <hammer/core/build_action.h>
#include <hammer/core/location.h>

namespace hammer{

class mksig_action : public build_action
{
   public:
      mksig_action();
      virtual std::string target_tag(const build_node& node, const build_environment& environment) const;

   protected:
      virtual bool execute_impl(const build_node& node, const build_environment& environment) const;
};

}

#endif //h_612363a8_96e9_4e1f_8fdd_706368d197d9
