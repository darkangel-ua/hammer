#if !defined(h_ff7955c3_3e98_46bd_bbe9_96c416a5134f)
#define h_ff7955c3_3e98_46bd_bbe9_96c416a5134f

#include <hammer/ast/expression.h>

namespace hammer{namespace ast{

class source_decl : public expression
{
   public:
      source_decl(const expression* sources) 
         : sources_(sources) 
      {}
      
      const expression* sources() const { return sources_; }
      virtual parscore::source_location start_loc() const { return sources_->start_loc(); }
      virtual bool accept(visitor& v) const;

   private:
      const expression* sources_;
};

}}

#endif //h_ff7955c3_3e98_46bd_bbe9_96c416a5134f
