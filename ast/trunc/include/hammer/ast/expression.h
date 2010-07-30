#if !defined(h_8babae10_1ba8_479f_ae25_e20b77eb41dc)
#define h_8babae10_1ba8_479f_ae25_e20b77eb41dc

#include <hammer/ast/statement.h>
#include <hammer/parscore/identifier.h>

namespace hammer{ namespace ast{

class expression : public statement
{
   public:
};

class empty_expr : public expression
{
   public:
      virtual bool accept(visitor& v) const;
};

class id_expr : public expression
{
   public:
      id_expr(const parscore::identifier& id) : id_(id) {}
      const parscore::identifier& id() const { return id_; } 

      virtual bool accept(visitor& v) const;
   
   private:
      parscore::identifier id_;
};

}}


#endif //h_8babae10_1ba8_479f_ae25_e20b77eb41dc
