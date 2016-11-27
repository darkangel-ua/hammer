#if !defined(h_15d0fd11_4781_476f_8b3d_693e36a98ae5)
#define h_15d0fd11_4781_476f_8b3d_693e36a98ae5

#include <hammer/ast/visitor.h>
#include <iosfwd>

namespace hammer{ namespace ast{

class ast_xml_printer : public visitor
{
   public:
      ast_xml_printer(std::ostream& os, 
                      bool write_header = true);
      bool visit(const hamfile& v) override;
      bool visit(const rule_invocation& v) override;
      bool visit(const list_of& v) override;
      bool visit(const requirement_set& v) override;
      bool visit(const error_expression& v) override;
      bool visit(const empty_expr& v) override;
      bool visit(const id_expr& v) override;
      bool visit(const named_expr& v) override;
      bool visit(const path_like_seq& v) override;
      bool visit(const simple_requirement& v) override;
      bool visit(const conditional_requirement& v) override;
      bool visit(const target_ref& v) override;
      bool visit(const feature& v) override;
      bool visit(const sources_decl& v);

   private:
      std::ostream& os_;
      unsigned indent_;
};

}}

#endif
