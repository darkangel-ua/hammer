#if !defined(h_15d0fd11_4781_476f_8b3d_693e36a98ae5)
#define h_15d0fd11_4781_476f_8b3d_693e36a98ae5

#include <hammer/ast/visitor.h>
#include <iosfwd>

namespace hammer{namespace ast{

class ast_xml_printer : public visitor
{
   public:
      ast_xml_printer(std::ostream& os, 
                      bool write_header = true);
      virtual bool visit_enter(const hamfile& v);
      virtual bool visit_leave(const hamfile& v);
      virtual bool visit_enter(const explicit_project_def& v);
      virtual bool visit_leave(const explicit_project_def& v);
      virtual bool visit(const implicit_project_def& v);
      virtual bool visit_enter(const rule_invocation& v);
      virtual bool visit_leave(const rule_invocation& v);
      virtual bool visit_enter(const list_of& v);
      virtual bool visit_leave(const list_of& v);
      virtual bool visit_enter(const requirement_set& v);
      virtual bool visit_leave(const requirement_set& v);
      virtual bool visit(const empty_expr& v);
      virtual bool visit(const id_expr& v);
      virtual bool visit(const path_like_seq& v);
      virtual bool visit(const simple_requirement& v);
      virtual bool visit(const conditional_requirement& v);
      virtual bool visit(const target_ref& v);
      virtual bool visit(const feature& v);

      virtual bool visit(const statements_t& v);
      virtual bool visit(const expressions_t& v);
      virtual bool visit(const requirements_t& v);
      virtual bool visit(const features_t& v); 
   
   private:
      std::ostream& os_;
      unsigned indent_;
};

}}
#endif //h_15d0fd11_4781_476f_8b3d_693e36a98ae5
