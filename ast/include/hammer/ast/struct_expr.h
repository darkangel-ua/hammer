#pragma once
#include <hammer/ast/expression.h>
#include <hammer/ast/types.h>

namespace hammer { namespace ast {

class struct_expr : public expression {
   public:
      struct_expr(const parscore::source_location& start_brace,
                  const expressions_t& fields)
         : start_brace_(start_brace),
           fields_(fields)
      {}

      parscore::source_location start_loc() const override { return start_brace_; }
      bool accept(visitor& v) const override;

      const parscore::source_location start_brace_;
      const expressions_t fields_;
};

}}
