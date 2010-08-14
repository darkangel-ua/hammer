#if !defined(h_56170038_85be_461a_817e_733c4bb21a2b)
#define h_56170038_85be_461a_817e_733c4bb21a2b

#include <hammer/ast/types.h>

namespace hammer{ namespace ast{
   class hamfile;
   class project_def;
   class rule_invocation;
   class parser_context;
   class path_like_seq;
   class requirement_set;
}}

namespace hammer{namespace parscore{
   class identifier;
   class source_location;
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
         on_named_expr(const parscore::identifier& name,
                       const ast::expression* value) const = 0;

      virtual const ast::expression* 
         on_list_of(const ast::expressions_t& e) const = 0;

      virtual const ast::path_like_seq* 
         on_path_like_seq(const parscore::identifier& first, 
                          const parscore::identifier& last) const = 0;
      
      virtual const ast::expression* 
         on_target_or_rule_call(const parscore::identifier& rule_name, 
                                const ast::expressions_t& arguments) const = 0;

      virtual const ast::requirement_set* 
         on_requirement_set(const ast::requirements_t& requirements) const = 0;

      virtual const ast::feature*
         on_feature(parscore::identifier name,
                    const ast::expression* value) const = 0;

      virtual const ast::requirement* 
         on_simple_requirement(parscore::source_location public_tag_loc,
                               const ast::feature* value) const = 0;

      virtual const ast::requirement* 
         on_conditional_requirement(parscore::source_location public_tag_loc,
                                    const ast::features_t& features,
                                    const ast::feature* value) const = 0;
      
      virtual const ast::expression*
         on_target_ref(parscore::source_location public_tag,
                       const ast::path_like_seq* head,
                       const parscore::identifier& target_name,
                       const ast::requirement_set* requirements) const = 0;
};


}}

#endif //h_56170038_85be_461a_817e_733c4bb21a2b
