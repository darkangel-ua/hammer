#if !defined(h_4de8fb35_0b58_4485_9979_9a85eec7db7d)
#define h_4de8fb35_0b58_4485_9979_9a85eec7db7d

#include <hammer/ast/rule_invocation.h>

namespace hammer{namespace ast{

class project_def : public rule_invocation
{
   public:
      project_def(const parscore::identifier& rule_name,
                  const expressions_t& arguments);

      virtual parscore::source_location start_loc() const { return rule_invocation::start_loc(); }
      bool accept(visitor& v) const override;
};

}}

#endif //h_4de8fb35_0b58_4485_9979_9a85eec7db7d
