#if !defined(h_bfff9dfc_78a7_4dd6_94c5_9033e1fd6b33)
#define h_bfff9dfc_78a7_4dd6_94c5_9033e1fd6b33

#include <hammer/ast/expression.h>
#include <hammer/ast/types.h>

namespace hammer{ namespace ast{

class path_like_seq : public expression
{
   public:
      path_like_seq(const parscore::identifier& first,
                    const parscore::identifier& last)
         : first_(first),
           last_(last)
      {
      }

      parscore::identifier to_identifier() const;
      virtual parscore::source_location start_loc() const { return first_.start_lok(); }
      virtual bool accept(visitor& v) const;
   
   private:
      parscore::identifier first_;
      parscore::identifier last_;
};

}}

#endif //h_bfff9dfc_78a7_4dd6_94c5_9033e1fd6b33
