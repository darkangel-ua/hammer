#pragma once
#include <hammer/parscore/identifier.h>
#include <hammer/ast/expression.h>
#include <hammer/ast/types.h>

namespace hammer { namespace ast {

class rule_arguments;

class rule_invocation : public expression {
   public:
      rule_invocation(const parscore::identifier& rule_name,
                      const expressions_t& arguments)
         : name_(rule_name),
           arguments_(arguments)
      {}

      const parscore::identifier& name() const { return name_; }
      const expressions_t& arguments() const { return arguments_; }

      parscore::source_location start_loc() const override { return name_.start_loc(); }
      bool accept(visitor& v) const override;

   private:
      parscore::identifier name_;
      expressions_t arguments_;
};

}}
