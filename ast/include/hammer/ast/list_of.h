#pragma once
#include <hammer/ast/expression.h>
#include <hammer/ast/types.h>

namespace hammer { namespace ast {

class list_of : public expression {
   public:
      list_of(const expressions_t& values)
         : values_(values)
      {}

      parscore::source_location start_loc() const override { return values_.front()->start_loc(); }
      bool accept(visitor& v) const override;
      const expressions_t& values() const { return values_; }
   
   private:
      expressions_t values_;
};

}}
