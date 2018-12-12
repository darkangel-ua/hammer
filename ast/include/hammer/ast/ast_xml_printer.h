#pragma once
#include <iosfwd>
#include <hammer/ast/visitor.h>

namespace hammer { namespace ast {

class ast_xml_printer : public visitor {
   public:
      ast_xml_printer(std::ostream& os, 
                      bool write_header = true);
      bool visit(const hamfile& v) override;
      bool visit(const rule_invocation& v) override;
      bool visit(const list_of& v) override;
      bool visit(const requirement_set& v) override;
		bool visit(const usage_requirements& v) override;
		bool visit(const error_expression& v) override;
      bool visit(const empty_expr& v) override;
      bool visit(const id_expr& v) override;
      bool visit(const named_expr& v) override;
		bool visit(const public_expr& v) override;
		bool visit(const path& v) override;
		bool visit(const target_ref& v) override;
		bool visit(const target_def& v) override;
		bool visit(const feature& v) override;
		bool visit(const feature_set& v) override;
		bool visit(const sources& v);
		bool visit(const condition_expr& v) override;
		bool visit(const logical_or& v) override;
		bool visit(const logical_and& v) override;
		bool visit(const expression_statement& v) override;
      bool visit(const struct_expr& v) override;

   private:
      std::ostream& os_;
      unsigned indent_;
};

}}
