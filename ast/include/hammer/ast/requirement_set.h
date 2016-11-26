#if !defined(h_3afdbfac_9e89_47b9_840a_f08a75ea6aa5)
#define h_3afdbfac_9e89_47b9_840a_f08a75ea6aa5

#include <hammer/ast/types.h>
#include <hammer/ast/expression.h>

namespace hammer{ namespace ast{

class requirement_set : public expression
{
   public:
      requirement_set(const requirements_t& requirements);
      
      const requirements_t& requirements() const { return requirements_; }
      virtual parscore::source_location start_loc() const;
      bool accept(visitor& v) const override;

   private:
      requirements_t requirements_;
};

}}

#endif
