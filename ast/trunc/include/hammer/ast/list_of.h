#if !defined(h_e2a33649_39e9_4f1b_ba94_b28252b20183)
#define h_e2a33649_39e9_4f1b_ba94_b28252b20183

#include <hammer/ast/expression.h>
#include <hammer/ast/types.h>

namespace hammer{ namespace ast{

class list_of : public expression
{
   public:
      list_of(const expressions_t& values)
         : values_(values)
      {}
      virtual bool accept(visitor& v) const;
   
   private:
      expressions_t values_;
};

}}

#endif //h_e2a33649_39e9_4f1b_ba94_b28252b20183
