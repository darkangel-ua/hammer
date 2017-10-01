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

namespace hammer{ namespace parscore{
   class identifier;
   class source_location;
}}

namespace hammer{ namespace sema{

class actions
{
   public:
      actions(ast::context& ctx) : ctx_(ctx) {}

      virtual const ast::hamfile*
         on_hamfile(const ast::statements_t& statements) const = 0;

      virtual const ast::expression*
         on_empty_expr(const parscore::identifier& next_token) const = 0;

      virtual const ast::expression*
         on_named_expr(const parscore::identifier& name,
                       const ast::expression* value) const = 0;

	  virtual const ast::expression*
		 on_public_expr(const parscore::identifier& tag,
						const ast::expression* value) const = 0;

      virtual const ast::expression*
         on_list_of(const ast::expressions_t& e) const = 0;

      virtual const ast::path_like_seq*
		 on_path_like_seq(const parscore::source_location root,
						  const ast::expressions_t& elements) const = 0;

	  virtual const ast::expression*
		 on_id(const parscore::identifier& id) const = 0;

	  virtual const ast::expression*
         on_target_or_rule_call(const parscore::identifier& rule_name,
                                const ast::expressions_t& arguments) const = 0;

      virtual const ast::feature*
         on_feature(parscore::identifier name,
                    const ast::expression* value) const = 0;

	  virtual const ast::expression*
         on_target_ref(parscore::source_location public_tag,
					   const ast::path_like_seq* target_path,
                       const parscore::identifier& target_name,
					   const ast::features_t& build_request) const = 0;
      ast::context& get_context() const { return ctx_; }

   protected:
      ast::context& ctx_;
};


}}

#endif //h_56170038_85be_461a_817e_733c4bb21a2b
