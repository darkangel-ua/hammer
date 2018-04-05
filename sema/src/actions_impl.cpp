#include <hammer/sema/actions_impl.h>
#include <hammer/ast/hamfile.h>
#include <hammer/ast/expression.h>
#include <hammer/ast/list_of.h>
#include <hammer/ast/sources.h>
#include <hammer/ast/path.h>
#include <hammer/ast/requirement_set.h>
#include <hammer/ast/usage_requirements.h>
#include <hammer/ast/rule_invocation.h>
#include <hammer/ast/target_ref.h>
#include <hammer/ast/target_def.h>
#include <hammer/ast/feature.h>
#include <hammer/ast/feature_set.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/condition.h>
#include <hammer/ast/target_def.h>
#include <hammer/core/diagnostic.h>
#include <set>

using namespace std;
using namespace hammer::ast;
using namespace hammer::parscore;

namespace hammer{namespace sema{

actions_impl::actions_impl(ast::context& ctx,
                           const hammer::rule_manager& rule_manager,
                           hammer::diagnostic& diag)
   : actions(ctx),
     rule_manager_(rule_manager),
     diag_(diag)
{
}

const ast::hamfile* 
actions_impl::on_hamfile(const ast::statements_t& statements)
{
   if (!statements.empty() &&
       is_a<expression_statement>(statements.front()) &&
       is_a<rule_invocation>(as<expression_statement>(statements.front())->content()) &&
       as<rule_invocation>(as<expression_statement>(statements.front())->content())->name() == "project")
   {
      const ast::statements_t st_without_project_def(statements.begin() + 1, statements.end(), statements.get_allocator());
      return new (ctx_) ast::hamfile(as<rule_invocation>(as<expression_statement>(statements.front())->content()), st_without_project_def);
   } else
      return new (ctx_) ast::hamfile(nullptr, statements);
}

const ast::expression* 
actions_impl::on_empty_expr(const parscore::identifier& next_token)
{
   return new (ctx_) ast::empty_expr(next_token);
}

const ast::expression* 
actions_impl::on_named_expr(const parscore::identifier& name,
                            const ast::expression* value)
{
   return new(ctx_) ast::named_expr(name, value);
}

const ast::expression*
actions_impl::on_public_expr(const parscore::identifier& tag,
                             const ast::expression* value)
{
   return new(ctx_) ast::public_expr(tag, value);
}

const ast::expression* 
actions_impl::on_list_of(const ast::expressions_t& e)
{
   return new (ctx_) ast::list_of(e);
}

const ast::path*
actions_impl::on_path(const token& root_name,
                      const expressions_t& elements,
                      const token& trailing_slash)
{
   return new (ctx_) ast::path(root_name, elements, trailing_slash);
}

const ast::expression*
actions_impl::on_id(const parscore::identifier& id)
{
   return new (ctx_) ast::id_expr(id);
}

const statement*
actions_impl::on_top_level_rule_invocation(const source_location explicit_tag,
                                           const source_location local_tag,
                                           const identifier& rule_name,
                                           const expressions_t& arguments)
{
   rule_manager::const_iterator i = rule_manager_.find(rule_name);
   if (i != rule_manager_.end()) {
      if (!first_rule_in_file_) {
         if (rule_name == "project") {
            diag_.error(rule_name.start_lok(), "Rule 'project' MUST be the first statement in the file");
            return new (ctx_) expression_statement(new (ctx_) ast::error_expression(rule_name.start_lok()));
         }
      } else
         first_rule_in_file_ = false;

      const rule_declaration& rd = i->second;
      const ast::rule_invocation* ri = new (ctx_) ast::rule_invocation(rule_name, process_arguments(rule_name, i->second, arguments));
      if (rd.is_target())
         return new (ctx_) target_def(explicit_tag, local_tag, ri);
      else {
         if (explicit_tag.valid()) {
            diag_.error(explicit_tag, "Only target definition can be explicit");
            return new (ctx_) expression_statement(new (ctx_) ast::error_expression(explicit_tag));
         } else if (local_tag.valid()) {
            diag_.error(local_tag, "Only target definition can be local");
            return new (ctx_) expression_statement(new (ctx_) ast::error_expression(local_tag));
         } else
            return new (ctx_) expression_statement(ri);
      }
   } else {
      diag_.error(rule_name.start_lok(), "Target or rule '%s' was not defined") << rule_name;
      return new (ctx_) expression_statement(new (ctx_) ast::error_expression(rule_name.start_lok()));
   }
}

static
int required_argument_count(const rule_declaration& rule_decl)
{
   int result = 0;
   for(rule_declaration::const_iterator i = rule_decl.begin() + 1, last = rule_decl.end(); i != last; ++i)
      if (!i->is_optional())
         ++result;

   return result;
}

const expression*
actions_impl::process_identifier_arg(const rule_argument& ra,
                                     const expression* arg)
{
   if (const id_expr* expr = as<id_expr>(arg))
      return expr;

   diag_.error(arg->start_loc(), "Argument '%s': must be simple identifier") << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_identifier_or_list_of_identifiers_arg(const rule_argument& ra,
                                                            const expression* arg)
{
   if (const id_expr* expr = as<id_expr>(arg))
      return expr;

   if (const list_of* l = as<list_of>(arg)) {
      for (const expression* e : l->values()) {
         if (!is_a<id_expr>(e)) {
            diag_.error(arg->start_loc(), "Argument '%s': must be simple identifier or list of identifiers") << ra.name();
            return new (ctx_) error_expression(arg);
         }
      }

      return arg;
   }

   diag_.error(arg->start_loc(), "Argument '%s': must be simple identifier or list of identifiers") << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_feature_set_arg(const rule_argument& ra,
                                      const expression* arg)
{
   if (is_a<ast::feature>(arg))
      return new (ctx_) ast::feature_set(arg);

   if (is_a<ast::list_of>(arg)) {
      for (const expression* e : as<list_of>(arg)->values()) {
         if (!is_a<ast::feature>(e)) {
            diag_.error(e->start_loc(), "Element of feature set should be simple features");
            return new (ctx_) error_expression(arg);
         }
      }

      return new (ctx_) ast::feature_set(arg);
   } else {
      diag_.error(arg->start_loc(), "Argument '%s': must be a feature set") << ra.name();
      return new (ctx_) error_expression(arg);
   }
}

const expression*
actions_impl::process_feature_arg(const rule_argument& ra,
                                  const expression* arg)
{
   if (is_a<ast::feature>(arg))
      return arg;

   diag_.error(arg->start_loc(), "Argument '%s': must be a feature") << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_sources_arg(const rule_argument& ra,
                                  const expression* arg)
{
   auto good_source = [](const expression* e) {
      return is_a<ast::id_expr>(e) || is_a<ast::target_ref>(e) || is_a<ast::path>(e) || is_a<ast::rule_invocation>(e);
   };

   auto process_rule_inv = [&](const ast::rule_invocation* ri) -> const expression* {
      // cannot fail because rule has been checked previously
      const rule_declaration& rd = rule_manager_.find(ri->name())->second;
      const rule_argument& rdr = rd.result();
      // we can deal only with return types that can be converted to sources
      if (rdr.type() == rule_argument_type::identifier ||
          rdr.type() == rule_argument_type::sources ||
          rdr.type() == rule_argument_type::path ||
          rdr.type() == rule_argument_type::target_ref)
      {
         return ri;
      }

      diag_.error(ri->start_loc(), "Can't use result of rule '%s' in sources") << rd.name();
      return new (ctx_) error_expression(ri);
   };

   if (good_source(arg)) {
      if (auto ri = as<ast::rule_invocation>(arg))
         return new (ctx_) hammer::ast::sources(process_rule_inv(ri));
       else
         return new (ctx_) hammer::ast::sources(arg);
   }

   if (is_a<ast::public_expr>(arg) && good_source(as<ast::public_expr>(arg)->value()))
      return new (ctx_) hammer::ast::sources(arg);

   if (is_a<ast::list_of>(arg)) {
      expressions_t elements(expressions_t::allocator_type{ctx_});
      for (const expression* e : as<list_of>(arg)->values()) {
         if (!good_source(e)) {
            diag_.error(e->start_loc(), "Bad source element");
            elements.push_back(new (ctx_) error_expression(arg));
         } else if (auto ri = as<ast::rule_invocation>(e))
            elements.push_back(process_rule_inv(ri));
         else
            elements.push_back(e);
      }

      return new (ctx_) hammer::ast::sources(new (ctx_) ast::list_of(elements));
   }

   diag_.error(arg->start_loc(), "Argument '%s': must meet sources specs") << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_requirements_decl_arg(const rule_argument& ra,
                                            const expression* arg)
{
   auto good_req = [] (const expression* e) {
      auto check = [] (const expression* e) { return is_a<ast::feature>(e) || is_a<condition_expr>(e); };
      return check(e) || (is_a<public_expr>(e) && check(as<public_expr>(e)->value()));
   };

   auto check_public = [this](const expression* e) {
      auto check_feature = [this] (const feature* f, bool is_public, bool in_condition) {
         if (const target_ref* tr = as<target_ref>(f->value())) {
            if (is_public && tr->public_tag().valid()) {
               if (in_condition)
                  diag_.error(tr->public_tag(), "Public conditional requirement result cannot contains feature with public target value");
               else
                  diag_.error(tr->public_tag(), "Public requirement feature cannot contains feature with public target value");
               return false;
            }
         }

         return true;
      };

      auto check = [this, &check_feature] (const expression* e, bool is_public) {
         if (const feature* f = as<feature>(e))
            return check_feature(f, is_public, false);
         else { // condition
            auto check_result_element = [this, &check_feature] (const expression* e, bool is_public) {
               if (const public_expr* p = as<public_expr>(e)) {
                  if (is_public) {
                     diag_.error(p->start_loc(), "Public conditional requirement result cannot contains public feature");
                     return false;
                  } else
                     return check_feature(as<feature>(p->value()), true, true);
               } else
                  return check_feature(as<feature>(e), is_public, true);
            };

            const condition_expr* c = as<condition_expr>(e);
            if (const list_of* l = as<list_of>(c->result())) {
               for (auto le : l->values()) {
                  if (!check_result_element(le, is_public))
                     return false;
               }
               return true;
            } else
               return check_result_element(c->result(), is_public);
         }
      };

      if (const public_expr* p = as<public_expr>(e))
         return check(p->value(), true);
      else
         return check(e, false);
   };

   if (is_a<list_of>(arg)) {
      for (const expression* e : as<list_of>(arg)->values()) {
         if (!good_req(e)) {
            diag_.error(e->start_loc(), "Requirement should be feature or condition");
            return new (ctx_) error_expression(arg);
         } else if (!check_public(e))
            return new (ctx_) error_expression(arg);
      }

      return new (ctx_) requirement_set(arg);
   } else if (good_req(arg)) {
      if (check_public(arg))
         return new (ctx_) requirement_set(arg);
      else
         return new (ctx_) error_expression(arg);
   } else {
      diag_.error(arg->start_loc(), "Requirement should be feature or condition");
      return new (ctx_) error_expression(arg);
   }
}

const expression*
actions_impl::process_usage_requirements_arg(const rule_argument& ra,
                                             const expression* arg)
{
   auto good_req = [] (const expression* e) {
      return is_a<ast::feature>(e) || is_a<condition_expr>(e);
   };

   if (is_a<list_of>(arg)) {
      for (const expression* e : as<list_of>(arg)->values()) {
         if (!good_req(e)) {
            diag_.error(arg->start_loc(), "Usage requirement should be feature or condition");
            return new (ctx_) error_expression(arg);
         }
      }

      return new (ctx_) ast::usage_requirements(arg);
   } else if (good_req(arg))
      return new (ctx_) ast::usage_requirements(arg);
   else {
      diag_.error(arg->start_loc(), "Usage requirement should be feature or condition");
      return new (ctx_) error_expression(arg);
   }
}

const expression*
actions_impl::process_path_like_seq_arg(const rule_argument& ra,
                                        const expression* arg)
{
   if (as<ast::path>(arg))
      return arg;

   diag_.error(arg->start_loc(), "Argument '%s': must be path like sequence") << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_feature_of_feature_set_arg(const rule_argument& ra,
                                                 const expression* arg)
{
   if (as<ast::feature>(arg))
      return process_feature_arg(ra, arg);
   else if (as<ast::list_of>(arg))
      return process_feature_set_arg(ra, arg);

   diag_.error(arg->start_loc(), "Argument '%s': expected feature or feature list") << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_path_or_list_of_paths(const rule_argument& ra,
                                            const expression* arg)
{
   if (as<ast::path>(arg))
      return arg;
   else if (const ast::list_of* l = ast::as<ast::list_of>(arg)) {
      for (auto le : l->values()) {
         if (!ast::as<path>(le)) {
            diag_.error(le->start_loc(), "Expected path") << ra.name();
            return new (ctx_) error_expression(arg);
         }
      }

      return arg;
   }

   diag_.error(arg->start_loc(), "Argument '%s': expected path or list of paths") << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_one_arg(const rule_argument& ra,
                              const expression* arg)
{
   if (!ra.is_optional() && is_a<empty_expr>(arg)) {
      diag_.error(as<empty_expr>(arg)->next_token().start_lok(), "Required argument '%s' expected before '%s'")
         << ra.name() 
         << as<empty_expr>(arg)->next_token();

      return new (ctx_) error_expression(arg->start_loc());
   }

   if (ra.is_optional() && is_a<empty_expr>(arg))
      return arg;

   switch(ra.type())
   {
      case rule_argument_type::identifier:
         return process_identifier_arg(ra, arg);

      case rule_argument_type::identifier_or_list_of_identifiers:
         return process_identifier_or_list_of_identifiers_arg(ra, arg);

      case rule_argument_type::feature:
         return process_feature_arg(ra, arg);

      case rule_argument_type::feature_set:
         return process_feature_set_arg(ra, arg);

      case rule_argument_type::sources:
         return process_sources_arg(ra, arg);

      case rule_argument_type::requirement_set:
         return process_requirements_decl_arg(ra, arg);

      case rule_argument_type::usage_requirements:
         return process_usage_requirements_arg(ra, arg);

      case rule_argument_type::path:
         return process_path_like_seq_arg(ra, arg);

      case rule_argument_type::ast_expression:
         return arg;

      case rule_argument_type::feature_or_feature_set:
         return process_feature_of_feature_set_arg(ra, arg);

      case rule_argument_type::path_or_list_of_paths:
         return process_path_or_list_of_paths(ra, arg);

      default:
         assert(false && "Unknown argument type");
         abort();
   }
}

expressions_t
actions_impl::process_arguments(const parscore::identifier& rule_name,
                                const rule_declaration& rule_decl,
                                const ast::expressions_t& arguments)
{
   typedef std::set<const rule_argument*> used_named_args_t;
   expressions_t result(expressions_t::allocator_type{ctx_});
   used_named_args_t used_named_args;

   bool only_named = false;
   // skip first argument because it always will be invocation_context
   // and second for targets because there always will be rule_target_attributes
   rule_declaration::const_iterator ra = rule_decl.begin() + 1;
   int required_argument_used = 0;
   for (expressions_t::const_iterator i = arguments.begin(), last = arguments.end(); i != last; ++i) {
      if (const named_expr* ne = as<named_expr>(*i)) {
         const identifier& arg_name = as<named_expr>(**i).name();
         rule_declaration::const_iterator r = rule_decl.find(arg_name);
         if (r == rule_decl.end()) {
            diag_.error(arg_name.start_lok(), "%s '%s' does not have named argument '%s'")
               << (rule_decl.is_target() ? "Target" : "Rule") 
               << rule_name 
               << arg_name;

            result.push_back(new (ctx_) error_expression((**i).start_loc()));
         } else { // named argument founded
            if (used_named_args.find(&*r) != used_named_args.end()) {
               diag_.error(arg_name.start_lok(), "%s '%s': argument '%s' used more than once")
                  << (rule_decl.is_target() ? "Target" : "Rule") 
                  << rule_name 
                  << arg_name;
               
               result.push_back(new (ctx_) error_expression((**i).start_loc()));
            } else {
               used_named_args.insert(&*r);
               if (!r->is_optional())
                  ++required_argument_used;

               const named_expr* new_ne = new (ctx_) named_expr(ne->name(), process_one_arg(*r, ne->value()));
               result.push_back(new_ne);
            }
         }

         only_named = true;
      } else { // unnamed argument
         // too many arguments
         if (ra == rule_decl.end()) {
            result.push_back(*i);
            ++required_argument_used;
            continue;
         }

         if (is_a<error_expression>(**i)) {
            if (!ra->is_optional())
               ++required_argument_used;
            
            result.push_back(*i);
         } else if (only_named) {
            diag_.error((**i).start_loc(), "Named argument expected");
            return result;
         } else {
            if (!ra->is_optional())
               ++required_argument_used;

            result.push_back(process_one_arg(*ra, *i));
         }
      }

      if (ra != rule_decl.end())
         ++ra;
   }
   
   int rac = required_argument_count(rule_decl);
   if (rac > required_argument_used) {
      diag_.error(rule_name.start_lok(), "%s '%s': not enough arguments")
         << (rule_decl.is_target() ? "Target" : "Rule") << rule_name;
   }

   if (ra == rule_decl.end() && rac != required_argument_used) {
      diag_.error(rule_name.start_lok(), "%s '%s': too many arguments")
         << (rule_decl.is_target() ? "Target" : "Rule") << rule_name;
   }

   return result;
}

const ast::expression*
actions_impl::on_rule_invocation(const parscore::identifier& rule_name,
                                 const ast::expressions_t& arguments)
{
   rule_manager::const_iterator i = rule_manager_.find(rule_name);
   if (i != rule_manager_.end()) {
      if (!first_rule_in_file_) {
         if (rule_name == "project")
            diag_.error(rule_name.start_lok(), "Rule 'project' MUST be the first statement in the file");
      } else
         first_rule_in_file_ = false;

      return new (ctx_) ast::rule_invocation(rule_name, process_arguments(rule_name, i->second, arguments));
   } else {
      diag_.error(rule_name.start_lok(), "Target or rule '%s' was not defined") << rule_name;
      return new (ctx_) ast::error_expression(rule_name.start_lok());
   }

   return new (ctx_) ast::rule_invocation(rule_name, arguments);
}

const ast::feature*
actions_impl::on_feature(parscore::identifier name,
                         const ast::expression* value)
{
   return new (ctx_) ast::feature(name, value);
}

const ast::expression*
actions_impl::on_target_ref(parscore::source_location public_tag,
                            const ast::path* target_path,
                            const parscore::identifier& target_name,
                            const features_t& build_request)
{
   return new (ctx_) ast::target_ref(public_tag, target_path, target_name, build_request);
}

const ast::logical_or*
actions_impl::on_logical_or(const ast::expression* left,
                            const ast::expression* right)
{
   return new (ctx_) ast::logical_or(left, right);
}

const ast::logical_and*
actions_impl::on_logical_and(const expression* left,
                             const expression* right)
{
   return new (ctx_) ast::logical_and(left, right);
}

const ast::condition_expr*
actions_impl::on_condition(const expression* condition,
                           const expression* result)
{
   return new (ctx_) ast::condition_expr(condition, result);
}

}}

