#if !defined(h_3afdbfac_9e89_47b9_840a_f08a75ea6aa5)
#define h_3afdbfac_9e89_47b9_840a_f08a75ea6aa5

#include <hammer/ast/types.h>
#include <hammer/ast/expression.h>

namespace hammer{ namespace ast{

class feature_set : public expression
{
   public:
      feature_set(const features_t& features) 
         : features_(features) 
      {}
      
      const features_t& features() const { return features_; }
      virtual bool accept(visitor& v) const;
      
   private:
      features_t features_;
};

}}
#endif //h_3afdbfac_9e89_47b9_840a_f08a75ea6aa5
