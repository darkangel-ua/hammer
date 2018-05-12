#pragma once
#include <hammer/ast/types.h>

namespace hammer { namespace ast {
   class hamfile;
   class project_def;
   class rule_invocation;
   class parser_context;
	class path;
   class requirement_set;
	class logical_or;
	class logical_and;
	class condition_expr;
}}

namespace hammer { namespace parscore {
   class identifier;
   class token;
   class source_location;
}}

namespace hammer { namespace sema {

class actions {
	public:
		actions(ast::context& ctx) : ctx_(ctx) {}

		virtual
		const ast::hamfile*
		on_hamfile(const ast::statements_t& statements) = 0;

		virtual
		const ast::expression*
		on_empty_expr(const parscore::identifier& next_token) = 0;

		virtual
		const ast::expression*
		on_named_expr(const parscore::identifier& name,
						  const ast::expression* value) = 0;

		virtual
		const ast::expression*
		on_public_expr(const parscore::identifier& tag,
							const ast::expression* value) = 0;

		virtual
		const ast::expression*
		on_list_of(const ast::expressions_t& e) = 0;

		virtual
		const ast::path*
		on_path(const parscore::token& root_name,
				  const ast::expressions_t& elements,
              const parscore::token& trailing_slash) = 0;

		virtual
		const ast::expression*
		on_id(const parscore::identifier& id) = 0;

		virtual
		const ast::expression*
		on_rule_invocation(const parscore::identifier& rule_name,
								 const ast::expressions_t& arguments) = 0;

		virtual
		const ast::statement*
		on_top_level_rule_invocation(const parscore::source_location explicit_tag,
											  const parscore::source_location local_tag,
											  const parscore::identifier& rule_name,
											  const ast::expressions_t& arguments) = 0;

		virtual
		const ast::feature*
		on_feature(parscore::identifier name,
					  const ast::expression* value) = 0;

		virtual
		const ast::logical_or*
		on_logical_or(const ast::expression* left,
						  const ast::expression* right) = 0;

		virtual
		const ast::logical_and*
		on_logical_and(const ast::expression* left,
							const ast::expression* right) = 0;

		virtual
		const ast::condition_expr*
		on_condition(const ast::expression* condition,
						 const ast::expression* result) = 0;

		virtual
		const ast::expression*
		on_target_ref(parscore::source_location public_tag,
						  const ast::path* target_path,
						  const parscore::identifier& target_name,
						  const ast::features_t& build_request) = 0;
		virtual ~actions() {}

		ast::context& get_context() const { return ctx_; }

	protected:
		ast::context& ctx_;
};

}}
