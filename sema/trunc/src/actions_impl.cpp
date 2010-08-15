#include "stdafx.h"
#include <hammer/sema/actions_impl.h>
#include <hammer/ast/hamfile.h>
#include <hammer/ast/project_def.h>
#include <hammer/ast/expression.h>
#include <hammer/ast/list_of.h>
#include <hammer/ast/path_like_seq.h>
#include <hammer/ast/requirement_set.h>
#include <hammer/ast/requirement.h>
#include <hammer/ast/target_ref.h>
#include <hammer/ast/feature.h>
#include <hammer/core/rule_manager.h>
#include <hammer/core/diagnostic.h>

namespace hammer{namespace sema{

actions_impl::actions_impl(ast::context& ctx)
   : ctx_(ctx)
{
}

void actions_impl::on_begin_parse(ast::parser_context* ctx) const
{
   ctx_.set_parser_context(std::auto_ptr<ast::parser_context>(ctx));
}

const ast::hamfile* 
actions_impl::on_hamfile(const ast::project_def* p,
                         const ast::statements_t& statements) const
{
   return new (ctx_) ast::hamfile(p, statements);
}

const ast::project_def* actions_impl::on_implicit_project_def() const
{
   return new (ctx_) ast::implicit_project_def();
}

const ast::project_def* 
actions_impl::on_explicit_project_def(const parscore::identifier& name, 
                                      const ast::expressions_t& expressions) const
{
   return new (ctx_) ast::explicit_project_def(name, expressions);
}

const ast::expression* 
actions_impl::on_empty_expr() const
{
   return new (ctx_) ast::empty_expr();
}

const ast::expression* 
actions_impl::on_named_expr(const parscore::identifier& name,
                            const ast::expression* value) const
{
   return new(ctx_) ast::named_expr(name, value);
}

const ast::expression* 
actions_impl::on_list_of(const ast::expressions_t& e) const
{
   return new (ctx_) ast::list_of(e);
}

const ast::path_like_seq* 
actions_impl::on_path_like_seq(const parscore::identifier& first, 
                               const parscore::identifier& last) const
{
   return new (ctx_) ast::path_like_seq(first, last);
}

const ast::expression* 
actions_impl::on_target_or_rule_call(const parscore::identifier& rule_name, 
                                     const ast::expressions_t& arguments) const
{
   rule_manager::const_iterator i = ctx_.rule_manager_.find(rule_name);
   if (i != ctx_.rule_manager_.end())
   {
//       if (i->arguments().size() != arguments.size())
//       {
//          ctx_.diag_.error("%s does not take %d arguments") << () ;
//          return &ctx_.error_expression_;
//       }
   }
   else
   {
      ctx_.diag_.error(rule_name.start_lok(), "Target or rule '%s' was not defined") << rule_name;      
      return new (ctx_) ast::error_expression();
   }

   return new (ctx_) ast::rule_invocation(rule_name, arguments);
}

const ast::requirement_set* 
actions_impl::on_requirement_set(const ast::requirements_t& requirements) const
{
   return new (ctx_) ast::requirement_set(requirements);
}

const ast::feature*
actions_impl::on_feature(parscore::identifier name,
                         const ast::expression* value) const
{
   return new (ctx_) ast::feature(name, value);
}

const ast::requirement*
actions_impl::on_simple_requirement(parscore::source_location public_tag_loc,
                                    const ast::feature* value) const
{
   return new (ctx_) ast::simple_requirement(public_tag_loc, value);
}

const ast::requirement* 
actions_impl::on_conditional_requirement(parscore::source_location public_tag_loc,
                                         const ast::features_t& features,
                                         const ast::feature* value) const
{
   return new (ctx_) ast::conditional_requirement(public_tag_loc, features, value);
}

const ast::expression*
actions_impl::on_target_ref(parscore::source_location public_tag,
                            const ast::path_like_seq* head,
                            const parscore::identifier& target_name,
                            const ast::requirement_set* requirements) const
{
   return new (ctx_) ast::target_ref(public_tag, head, target_name, requirements);
}

}}
