#if !defined(h_56170038_85be_461a_817e_733c4bb21a2b)
#define h_56170038_85be_461a_817e_733c4bb21a2b

#include <hammer/ast/types.h>

namespace hammer{ namespace ast{
   class hamfile;
   class project_def;
   class rule_invocation;
   class expression;
   class parser_context;
   class statement;
}}

namespace hammer{namespace parscore{
   class identifier;
}}

namespace hammer{namespace sema{

class actions
{
   public:
      virtual void on_begin_parse(ast::parser_context* ctx) const = 0;
      virtual const ast::hamfile* 
         on_hamfile(const ast::project_def* p,
                    const ast::statements_t& statements) const = 0;
      
      virtual const ast::project_def* 
         on_implicit_project_def() const = 0;
      
      virtual const ast::project_def* 
         on_explicit_project_def(const parscore::identifier& name, 
                                 const ast::expressions_t& expressions) const = 0;
      virtual const ast::expression* 
         on_empty_expr() const = 0;

      virtual const ast::expression* 
         on_list_of(const ast::expressions_t& e) const = 0;

      virtual const ast::expression* 
         on_path_like_seq(const parscore::identifier& first, 
                          const parscore::identifier& last) const = 0;
      
      virtual const ast::expression* 
         on_target_or_rule_call(const parscore::identifier& rule_name, 
                                const ast::expressions_t& arguments) const = 0;
};


}}

#endif //h_56170038_85be_461a_817e_733c4bb21a2b
