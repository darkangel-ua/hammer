#pragma once
#include <hammer/ast/expression.h>

namespace hammer { namespace ast {

class feature : public expression {
   public:
      feature(const parscore::identifier& name, 
              const expression* value)
         : name_(name),
           value_(value)
      {}

      const parscore::identifier& name() const { return name_; }
      const expression* value() const { return value_; }
      parscore::source_location start_loc() const override { return name_.start_loc(); }
      bool accept(visitor& v) const override;

   private:
      parscore::identifier name_;
      const expression* value_;
};

}}
