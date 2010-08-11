#if !defined(h_a2e1a047_b168_45ff_978b_efac35d5573d)
#define h_a2e1a047_b168_45ff_978b_efac35d5573d

#include <hammer/ast/expression.h>

namespace hammer{namespace ast{

class feature : public expression
{
   public:
      feature(const parscore::identifier& name, 
              const expression* value)
         : name_(name),
           value_(value)
      {}

      const parscore::identifier& name() const { return name_; }
      const expression* value() const { return value_; }
      virtual bool accept(visitor& v) const;
   
   private:
      parscore::identifier name_;
      const expression* value_;
};

}}

#endif //h_a2e1a047_b168_45ff_978b_efac35d5573d
