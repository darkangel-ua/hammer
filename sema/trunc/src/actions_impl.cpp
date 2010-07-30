#include "stdafx.h"
#include <hammer/sema/actions_impl.h>
#include <hammer/ast/hamfile.h>
#include <hammer/ast/project_def.h>
#include <hammer/ast/expression.h>
#include <hammer/ast/list_of.h>
#include <hammer/ast/path_like_seq.h>

namespace hammer{namespace sema{

actions_impl::actions_impl(ast::context& ctx)
   : ctx_(ctx)
{
}

void actions_impl::on_begin_parse(ast::parser_context* ctx) const
{
   ctx_.set_parser_context(std::auto_ptr<ast::parser_context>(ctx));
}

const ast::hamfile* actions_impl::on_hamfile(const ast::project_def* p) const
{
   return new (ctx_) ast::hamfile(p, ast::statements_t());
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
actions_impl::on_list_of(const ast::expressions_t& e) const
{
   return new (ctx_) ast::list_of(e);
}

const ast::expression* 
actions_impl::on_path_like_seq(const parscore::identifier& first, 
                               const parscore::identifier& last) const
{
   return new (ctx_) ast::path_like_seq(first, last);
}

}}
