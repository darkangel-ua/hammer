#if !defined(h_66fabe72_d8a5_489a_a602_95149c6b80c4)
#define h_66fabe72_d8a5_489a_a602_95149c6b80c4

#include <hammer/ast/expression.h>

namespace hammer{ namespace ast{

class feature : public expression
{
};

class simple_feature : public feature
{
   public:
      simple_feature(const parscore::identifier& name, 
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

class conditional_feature : public feature
{
   public:
      conditional_feature(const features_t& features, 
                          const expression* value)
        : features_(features),
          value_(value)
      {}
      
      const features_t& features() const { return features_; }
      const expression* value() const { return value_; }
      virtual bool accept(visitor& v) const;

   private:
      features_t features_;
      const expression* value_;
};

}}
#endif //h_66fabe72_d8a5_489a_a602_95149c6b80c4
