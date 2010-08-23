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
#include <hammer/ast/casts.h>
#include <hammer/core/rule_manager.h>
#include <hammer/core/diagnostic.h>

using namespace std;
using namespace hammer::ast;
using namespace hammer::parscore;

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
   return new (ctx_) ast::empty_expr(parscore::source_location());
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

static int required_argument_count(const rule_declaration& rule_decl)
{
   int result = 0;
   for(rule_declaration::const_iterator i = rule_decl.begin(), last = rule_decl.end(); i != last; ++i)
      if (!i->is_optional())
         ++result;

   return result;
}

static
expressions_t process_arguments(const parscore::identifier& rule_name,
                                const rule_declaration& rule_decl, 
                                const ast::expressions_t& arguments,
                                ast::context& ctx)
{
   typedef vector<pair<const expression*, const rule_argument*> > processed_arguments_t;
   expressions_t result;
   processed_arguments_t pargs;
   bool only_named = false;
   rule_declaration::const_iterator ra = rule_decl.begin();
   int required_argument_used = 0;
   for(expressions_t::const_iterator i = arguments.begin(), last = arguments.end(); i != last; ++i)
   {
      if (is_a<named_expr>(**i))
      {
         const identifier& arg_name = as<named_expr>(**i).name();
         rule_declaration::const_iterator r = rule_decl.find(arg_name);
         if (r == rule_decl.end())
         {
            ctx.diag_.error(arg_name.start_lok(), "%s '%s' does not have named argument '%s'") 
               << (rule_decl.is_target() ? "Target" : "Rule") 
               << rule_name 
               << arg_name;

            result.push_back(new (ctx) error_expression((**i).start_loc()));
         }
         else
         {
            if (!r->is_optional())
               ++required_argument_used;

            result.push_back(*i);
         }

         only_named = true;
      }
      else
      {
         // too many arguments
         if (ra == rule_decl.end())
         {
            result.push_back(*i);
            ++required_argument_used;
            continue;
         }

         if (is_a<error_expression>(**i))
         {
            if (!ra->is_optional())
               ++required_argument_used;
            
            result.push_back(*i);
         }
         else
            if (only_named)
            {
               ctx.diag_.error((**i).start_loc(), "Named argument expected");
               return result;
            }
            else
            {
               if (!ra->is_optional())
                  ++required_argument_used;

               result.push_back(*i);
            }
      }

      if (ra != rule_decl.end())
         ++ra;
   }
   
   int rac = required_argument_count(rule_decl);
   if (rac > required_argument_used)
   {
      ctx.diag_.error(rule_name.start_lok(), "%s '%s': not enough arguments")
         << (rule_decl.is_target() ? "Target" : "Rule") << rule_name;
   }

   if (ra == rule_decl.end() && rac != required_argument_used)
   {
      ctx.diag_.error(rule_name.start_lok(), "%s '%s': too many arguments")
         << (rule_decl.is_target() ? "Target" : "Rule") << rule_name;
   }

   return result;
}

const ast::expression* 
actions_impl::on_target_or_rule_call(const parscore::identifier& rule_name, 
                                     const ast::expressions_t& arguments) const
{
   rule_manager::const_iterator i = ctx_.rule_manager_.find(rule_name);
   if (i != ctx_.rule_manager_.end())
      return new (ctx_) ast::rule_invocation(rule_name, process_arguments(rule_name, i->second, arguments, ctx_));
   else
   {
      ctx_.diag_.error(rule_name.start_lok(), "Target or rule '%s' was not defined") << rule_name;      
      return new (ctx_) ast::error_expression(rule_name.start_lok());
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
