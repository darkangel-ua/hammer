#if !defined(h_66fabe72_d8a5_489a_a602_95149c6b80c4)
#define h_66fabe72_d8a5_489a_a602_95149c6b80c4

#include <hammer/ast/expression.h>

namespace hammer{ namespace ast{

class feature;

class requirement : public expression
{
   public:
      requirement(parscore::source_location public_tag)
         : public_tag_(public_tag)
      {}
      
      parscore::source_location public_tag() const { return public_tag_; }
      bool is_public() const { return public_tag().valid(); }

   private:
      parscore::source_location public_tag_;
};

class simple_requirement : public requirement
{
   public:
      simple_requirement(parscore::source_location public_tag_loc,
                         const feature* value)
         : requirement(public_tag_loc),
           value_(value)
      {}

      const feature* value() const { return value_; }
      virtual bool accept(visitor& v) const;
   
   private:
      const feature* value_;
};

class conditional_requirement : public requirement
{
   public:
      conditional_requirement(parscore::source_location public_tag_loc, 
                              const features_t& features, 
                              const feature* value)
        : requirement(public_tag_loc),
          features_(features),
          value_(value)
      {}
      
      const features_t& features() const { return features_; }
      const feature* value() const { return value_; }
      virtual bool accept(visitor& v) const;

   private:
      features_t features_;
      const feature* value_;
};

}}
#endif //h_66fabe72_d8a5_489a_a602_95149c6b80c4
