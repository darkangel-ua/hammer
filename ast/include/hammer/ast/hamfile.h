#pragma once
#include <vector>
#include <hammer/ast/node.h>
#include <hammer/ast/types.h>

namespace hammer { namespace ast {

class rule_invocation;

class hamfile : public node {
   public:
      hamfile(const rule_invocation* project_def,
              const statements_t& s)
         : project_def_(project_def),
           statements_(s)
      {}

      bool accept(visitor& v) const override;

      const rule_invocation* get_project_def() const { return project_def_; }
      const statements_t& get_statements() const { return statements_; }

   private:
      const rule_invocation* project_def_;
      statements_t statements_;
};

}}
