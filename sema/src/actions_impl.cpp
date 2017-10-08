#include "stdafx.h"
#include <hammer/sema/actions_impl.h>
#include <hammer/ast/hamfile.h>
#include <hammer/ast/expression.h>
#include <hammer/ast/list_of.h>
#include <hammer/ast/sources_decl.h>
#include <hammer/ast/path.h>
#include <hammer/ast/requirement_set.h>
#include <hammer/ast/rule_invocation.h>
#include <hammer/ast/target.h>
#include <hammer/ast/feature.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/condition.h>
#include <hammer/core/rule_manager.h>
#include <hammer/core/diagnostic.h>
#include <set>

using namespace std;
using namespace hammer::ast;
using namespace hammer::parscore;

namespace hammer{namespace sema{

static
expressions_t process_arguments(const parscore::identifier& rule_name,
                                const rule_declaration& rule_decl,
                                const ast::expressions_t& arguments,
                                ast::context& ctx);

actions_impl::actions_impl(ast::context& ctx)
   : actions(ctx)
{
}

const ast::hamfile* 
actions_impl::on_hamfile(const ast::statements_t& statements) const
{
   if (!statements.empty() &&
       is_a<rule_invocation>(statements.front()) &&
       as<rule_invocation>(statements.front())->name() == "project")
   {
      const ast::statements_t st_without_project_def(statements.begin() + 1, statements.end(), statements.get_allocator());
      return new (ctx_) ast::hamfile(as<rule_invocation>(statements.front()), st_without_project_def);
   } else
      return new (ctx_) ast::hamfile(nullptr, statements);
}

const ast::expression* 
actions_impl::on_empty_expr(const parscore::identifier& next_token) const
{
   return new (ctx_) ast::empty_expr(next_token);
}

const ast::expression* 
actions_impl::on_named_expr(const parscore::identifier& name,
                            const ast::expression* value) const
{
   return new(ctx_) ast::named_expr(name, value);
}

const ast::expression*
actions_impl::on_public_expr(const parscore::identifier& tag,
                             const ast::expression* value) const
{
   return new(ctx_) ast::public_expr(tag, value);
}

const ast::expression* 
actions_impl::on_list_of(const ast::expressions_t& e) const
{
   return new (ctx_) ast::list_of(e);
}

const ast::path*
actions_impl::on_path(const source_location root,
                      const expressions_t& elements) const
{
   return new (ctx_) ast::path(root, elements);
}

const ast::expression*
actions_impl::on_id(const parscore::identifier& id) const
{
   return new (ctx_) ast::id_expr(id);
}

static int required_argument_count(const rule_declaration& rule_decl)
{
   int result = 0;
   for(rule_declaration::const_iterator i = rule_decl.begin(), last = rule_decl.end(); i != last; ++i)
      if (!i->is_optional())
         ++result;

   return result;
}

static const expression*
process_identifier_arg(const rule_argument& ra, const expression* arg, context& ctx)
{
   if (const id_expr* expr = as<id_expr>(arg))
      return expr;

   ctx.diag_.error(arg->start_loc(), "Argument '%s': must be simple identifier") << ra.name();
   return new (ctx) error_expression(arg);
}

static const expression*
process_feature_set_arg(const rule_argument& ra, const expression* arg, context& ctx)
{
   return new (ctx) error_expression(arg);
}

static const expression*
process_sources_decl_arg(const rule_argument& ra, const expression* arg, context& ctx)
{
   // FIXME: semantic checks
   return new (ctx) hammer::ast::sources_decl(arg);
}

static const expression*
process_requirements_decl_arg(const rule_argument& ra, const expression* arg, context& ctx)
{
   auto good_req = [] (const expression* e) {
      auto check = [] (const expression* e) { return is_a<feature>(e) || is_a<condition_expr>(e); };
      return check(e) || (is_a<public_expr>(e) && check(as<public_expr>(e)->value()));
   };

   if (is_a<list_of>(arg)) {
      for (const expression* e : as<list_of>(arg)->values()) {
         if (!good_req(e)) {
            ctx.diag_.error(e->start_loc(), "Requirement should be feature or condition");
            return new (ctx) error_expression(arg);
         }
      }

      return new (ctx) requirement_set(arg);
   } else if (good_req(arg))
      return new (ctx) requirement_set(arg);
   else {
      ctx.diag_.error(arg->start_loc(), "Requirement should be feature or condition");
      return new (ctx) error_expression(arg);
   }
}

static const expression*
process_path_like_seq_arg(const rule_argument& ra, const expression* arg, context& ctx)
{
   if (as<ast::path>(arg))
      return arg;

   ctx.diag_.error(arg->start_loc(), "Argument '%s': must be path like sequence") << ra.name();
   return new (ctx) error_expression(arg);
}

static const expression* 
process_one_arg(const rule_argument& ra, const expression* arg, ast::context& ctx)
{
   if (!ra.is_optional() && is_a<empty_expr>(arg))
   {
      ctx.diag_.error(arg->start_loc(), "Required argument '%s' expected before '%s'")
         << ra.name() 
         << as<empty_expr>(arg)->next_token();

      return new (ctx) error_expression(arg->start_loc());
   }

   if (ra.is_optional() && is_a<empty_expr>(arg))
      return arg;

   switch(ra.type())
   {
      case rule_argument_type::IDENTIFIER:
         return process_identifier_arg(ra, arg, ctx);

      case rule_argument_type::FEATURE_SET:
         return process_feature_set_arg(ra, arg, ctx);

      case rule_argument_type::SOURCES_SET:
         return process_sources_decl_arg(ra, arg, ctx);

      case rule_argument_type::REQUIREMENTS_SET:
         return process_requirements_decl_arg(ra, arg, ctx);

      case rule_argument_type::PATH:
         return process_path_like_seq_arg(ra, arg, ctx);

      default:
         assert(false && "Unknown argument type");
         abort();
         throw;
   }
}

static
expressions_t process_arguments(const parscore::identifier& rule_name,
                                const rule_declaration& rule_decl, 
                                const ast::expressions_t& arguments,
                                ast::context& ctx)
{
   typedef std::set<const rule_argument*> used_named_args_t;
   expressions_t result(expressions_t::allocator_type{ctx});
   used_named_args_t used_named_args;

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
         else // named argument founded
         {
            if (used_named_args.find(&*r) != used_named_args.end())
            {
               ctx.diag_.error(arg_name.start_lok(), "%s '%s': named argument '%s' used more than once") 
                  << (rule_decl.is_target() ? "Target" : "Rule") 
                  << rule_name 
                  << arg_name;
               
               result.push_back(new (ctx) error_expression((**i).start_loc()));
            }
            else 
            {
               used_named_args.insert(&*r);
               if (!r->is_optional())
                  ++required_argument_used;

               result.push_back(*i);
            }
         }

         only_named = true;
      }
      else // unnamed argument
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

               result.push_back(process_one_arg(*ra, *i, ctx));
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
   if (i != ctx_.rule_manager_.end()) {
      if (!first_rule_in_file_) {
         if (rule_name == "project")
            ctx_.diag_.error(rule_name.start_lok(), "Rule 'project' MUST be the first statement in the file");
      } else
         first_rule_in_file_ = false;

      return new (ctx_) ast::rule_invocation(rule_name, process_arguments(rule_name, i->second, arguments, ctx_));
   } else {
      ctx_.diag_.error(rule_name.start_lok(), "Target or rule '%s' was not defined") << rule_name;
      return new (ctx_) ast::error_expression(rule_name.start_lok());
   }

   return new (ctx_) ast::rule_invocation(rule_name, arguments);
}

const ast::feature*
actions_impl::on_feature(parscore::identifier name,
                         const ast::expression* value) const
{
   return new (ctx_) ast::feature(name, value);
}

const ast::expression*
actions_impl::on_target(parscore::source_location public_tag,
                        const ast::path* target_path,
                        const parscore::identifier& target_name,
                        const features_t& build_request) const
{
   return new (ctx_) ast::target(public_tag, target_path, target_name, build_request);
}

const ast::logical_or*
actions_impl::on_logical_or(const ast::expression* left,
                            const ast::expression* right) const
{
   return new (ctx_) ast::logical_or(left, right);
}

const ast::logical_and*
actions_impl::on_logical_and(const expression* left,
                             const expression* right) const
{
   return new (ctx_) ast::logical_and(left, right);
}

const ast::condition_expr*
actions_impl::on_condition(const expression* condition,
                           const expression* result) const
{
   return new (ctx_) ast::condition_expr(condition, result);
}

}}

