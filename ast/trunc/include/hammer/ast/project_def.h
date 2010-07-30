#if !defined(h_4de8fb35_0b58_4485_9979_9a85eec7db7d)
#define h_4de8fb35_0b58_4485_9979_9a85eec7db7d

#include <hammer/ast/rule_invocation.h>

namespace hammer{namespace ast{

class project_def : public expression
{
   public:
};

class implicit_project_def : public project_def
{
   public:
      virtual bool accept(visitor& v) const;
};

class explicit_project_def : public project_def,
                             public rule_invocation
{
   public:
      explicit_project_def(const parscore::identifier& rule_name,
                           const expressions_t& arguments);
      virtual bool accept(visitor& v) const;
};

}}

#endif //h_4de8fb35_0b58_4485_9979_9a85eec7db7d
