#if !defined(h_484613f1_a4c9_4315_8c2d_b4bb1421f61b)
#define h_484613f1_a4c9_4315_8c2d_b4bb1421f61b

#include <hammer/ast/types.h>

namespace hammer{namespace ast{

class hamfile;
class explicit_project_def;
class implicit_project_def;
class rule_invocation;
class rule_arguments;
class empty_expr;
class named_expr;
class id_expr;
class path_like_seq;
class list_of;
class requirement_set;
class simple_requirement;
class conditional_requirement;
class target_ref;
class feature;

class visitor
{
   public:
      virtual bool visit_enter(const hamfile& v) { return true; }
      virtual bool visit_leave(const hamfile& v) { return true; }
      virtual bool visit_enter(const explicit_project_def& v) { return true; }
      virtual bool visit_leave(const explicit_project_def& v) { return true; }
      virtual bool visit(const implicit_project_def& v) { return true; }
      virtual bool visit_enter(const rule_invocation& v) { return true; }
      virtual bool visit_leave(const rule_invocation& v) { return true; }
      virtual bool visit_enter(const list_of& v) { return true; }
      virtual bool visit_leave(const list_of& v) { return true; }
      virtual bool visit_enter(const requirement_set& v) { return true; }
      virtual bool visit_leave(const requirement_set& v) { return true; }
      virtual bool visit(const empty_expr& v) { return true; }
      virtual bool visit(const id_expr& v) { return true; }
      virtual bool visit(const named_expr& v) { return true; }
      virtual bool visit(const path_like_seq& v) { return true; }
      virtual bool visit(const simple_requirement& v) { return true; }
      virtual bool visit(const conditional_requirement& v) { return true; }
      virtual bool visit(const target_ref& v) { return true; }
      virtual bool visit(const feature& v) { return true; }
      
      // FIXME: May this is wrong and we should make statements separate class?
      virtual bool visit(const statements_t& v) { return true; } 
      virtual bool visit(const expressions_t& v) { return true; } 
      virtual bool visit(const requirements_t& v) { return true; } 
      virtual bool visit(const features_t& v) { return true; } 
};

}}
#endif //h_484613f1_a4c9_4315_8c2d_b4bb1421f61b
