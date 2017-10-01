#if !defined(h_7858a0f2_780c_4919_9a5a_86104e12c129)
#define h_7858a0f2_780c_4919_9a5a_86104e12c129

#include <hammer/sema/actions.h>
#include <hammer/ast/context.h>

namespace hammer{ namespace sema{

class actions_impl : public actions
{
   public:
      actions_impl(ast::context& ctx);
      
      virtual const ast::hamfile* 
         on_hamfile(const ast::statements_t& statements) const;
      
      virtual const ast::expression* 
         on_empty_expr(const parscore::identifier& next_token) const;

      virtual const ast::expression* 
         on_named_expr(const parscore::identifier& name,
                       const ast::expression* value) const;

	  const ast::expression*
	  on_public_expr(const parscore::identifier& tag,
					 const ast::expression* value) const override;

	  virtual const ast::expression*
         on_list_of(const ast::expressions_t& e) const;

      virtual const ast::path_like_seq* 
		 on_path_like_seq(const parscore::source_location root,
						  const ast::expressions_t& elements) const;

	  virtual const ast::expression*
		 on_id(const parscore::identifier& id) const;

	  virtual const ast::expression*
         on_target_or_rule_call(const parscore::identifier& rule_name, 
                                const ast::expressions_t& arguments) const;

      virtual const ast::feature*
         on_feature(parscore::identifier name,
                    const ast::expression* value) const;

      virtual const ast::expression*
		  on_target(parscore::source_location public_tag,
					   const ast::path_like_seq* target_path,
					   const parscore::identifier& target_name,
					   const ast::features_t& build_request) const;
   private:
      mutable bool first_rule_in_file_ = true;
};

}}

#endif
