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

namespace hammer { namespace sema {

actions_impl::actions_impl(ast::context& ctx,
                           environment& env,
                           const hammer::rule_manager& rule_manager,
                           hammer::diagnostic& diag)
   : actions(ctx),
     env_(env),
     rule_manager_(rule_manager),
     diag_(diag)
{
   assert(rule_manager.find("feature") != rule_manager.end() && "Feature rule must be defined");
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
            diag_.error(rule_name.start_loc(), "Rule 'project' MUST be the first statement in the file");
            return new (ctx_) expression_statement(new (ctx_) ast::error_expression(rule_name.start_loc()));
         }
      } else
         first_rule_in_file_ = false;

      const auto error_count = diag_.error_count();
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
         } else {
            // add feature to set of defined features if no error occured in arguments processing
            if (rule_name == "feature" && error_count == diag_.error_count()) {
               assert(as<id_expr>(arguments[0]) && "First argument of feature rule should be id_expr");
               const parscore::identifier& feature_name = as<id_expr>(arguments[0])->id();
               if (known_feature(feature_name)) {
                  diag_.error(rule_name.start_loc(), "Feature '%s' already defined") << feature_name;
                  return new (ctx_) expression_statement(new (ctx_) ast::error_expression(ri));
               } else
                  features_.insert(feature_name.to_string());
            }

            return new (ctx_) expression_statement(ri);
         }
      }
   } else {
      diag_.error(rule_name.start_loc(), "Target or rule '%s' was not defined") << rule_name;
      return new (ctx_) expression_statement(new (ctx_) ast::error_expression(rule_name.start_loc()));
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
   auto process_one = [&](const expression* e) -> const expression* {
      if (const feature* f = as<feature>(e)) {
         if (!known_feature(f->name())) {
            diag_.error(f->start_loc(), "Argument '%s': Unknown feature '%s'") << ra.name() << f->name();
            return new (ctx_) error_expression(e);
         } else
            return e;
      } else {
         diag_.error(e->start_loc(), "Argument '%s': Feature set elements are simple features") << ra.name();
         return new (ctx_) error_expression(e);
      }
   };

   if (const list_of* l = as<list_of>(arg)) {
      expressions_t elements{expressions_t::allocator_type{ctx_}};
      for (const expression* le : l->values())
         elements.push_back(process_one(le));

      return new (ctx_) feature_set{new (ctx_) list_of{elements}};
   } else
      return new (ctx_) feature_set{process_one(arg)};
}

const expression*
actions_impl::process_feature_arg(const rule_argument& ra,
                                  const expression* arg)
{
   if (const feature* f = ast::as<feature>(arg)) {
      if (!known_feature(f->name())) {
         diag_.error(arg->start_loc(), "Argument '%s': Unknown feature '%s'") << ra.name() << f->name();
         return new (ctx_) error_expression(arg);
      } else
         return arg;
   }

   diag_.error(arg->start_loc(), "Argument '%s': feature expected") << ra.name();
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
   auto wrap_public = [&](const expression* e,
                          const public_expr* pe) -> const expression* {
      if (pe)
         return new (ctx_) public_expr{pe->tag(), e};
      else
         return e;
   };

   auto process_result_element = [&](const expression* e,
                                     const bool public_requirement) -> const expression* {
      const public_expr* pe = as<public_expr>(e);
      if (pe) {
         if (public_requirement) {
            diag_.error(e->start_loc(), "Argument '%s': Public feature is not allowed here") << ra.name();
            return new (ctx_) error_expression(e);
         } else
            e = pe->value();
      }

      if (const feature* f = as<feature>(e)) {
         if (!known_feature(f->name())) {
            diag_.error(f->start_loc(), "Argument '%s': Unknown feature '%s'") << ra.name() << f->name();
            return new (ctx_) error_expression(e);
         } else if (const target_ref* tr = as<target_ref>(f->value())) {
            if (tr->is_public() && public_requirement) {
               diag_.error(f->value()->start_loc(), "Argument '%s': Public target is not allowed here") << ra.name();
               return new (ctx_) error_expression(e);
            }
         }
         return wrap_public(e, pe);
      } else {
         diag_.error(e->start_loc(), "Argument '%s': Expected feature or condition") << ra.name();
         return new (ctx_) error_expression(e);
      }
   };

   auto process_one_requirement = [&](const expression* e) -> const expression* {
      bool public_requirement = false;
      const public_expr* pe = as<public_expr>(e);
      if (pe) {
         e = pe->value();
         public_requirement = true;
      }

      if (const condition_expr* ce = as<condition_expr>(e)) {
         const expression* condition = process_condition(ra, ce->condition());
         if (const list_of* l = as<list_of>(ce->result())) {
            expressions_t values{expressions_t::allocator_type{ctx_}};
            for (const expression* le : l->values())
               values.push_back(process_result_element(le, public_requirement));

            return wrap_public(new (ctx_) condition_expr(condition, new (ctx_) list_of(values)), pe);
         } else
            return wrap_public(new (ctx_) condition_expr(condition, process_result_element(ce->result(), public_requirement)), pe);
      } else
         return wrap_public(process_result_element(e, public_requirement), pe);
   };

   if (const list_of* l = as<list_of>(arg)) {
      expressions_t values{expressions_t::allocator_type{ctx_}};
      for (const expression* le : l->values())
         values.push_back(process_one_requirement(le));

      return new (ctx_) requirement_set{new (ctx_) list_of(values)};
   } else
      return new (ctx_) requirement_set(process_one_requirement(arg));
}

const ast::expression*
actions_impl::process_condition(const rule_argument& ra,
                                const ast::expression* e)
{
   if (const ast::logical_or* lo = as<ast::logical_or>(e))
      return new (ctx_) ast::logical_or(process_condition(ra, lo->left()), process_condition(ra, lo->right()));
   else if (const ast::logical_and* la = as<ast::logical_and>(e))
      return new (ctx_) ast::logical_and(process_condition(ra, la->left()), process_condition(ra, la->right()));
   else if (const feature* f = as<feature>(e)) {
      if (!known_feature(f->name())) {
         diag_.error(f->start_loc(), "Argument '%s': Unknown feature '%s'") << ra.name() << f->name();
         return new (ctx_) error_expression(e);
      }
      return f;
   } else {
      // this can't happens because parser will not allow this, but I added this just in case :)
      diag_.error(e->start_loc(), "Argument '%s': Expected feature or condition expression");
      return new (ctx_) error_expression(e);
   }
}

const expression*
actions_impl::process_usage_requirements_arg(const rule_argument& ra,
                                             const expression* arg)
{
   auto process_result_element = [&](const expression* e) -> const expression* {
      if (as<public_expr>(e)) {
         diag_.error(e->start_loc(), "Argument '%s': Public expressions is not allowed here") << ra.name();
         return new (ctx_) error_expression(e);
      } else if (const feature* f = as<feature>(e)) {
         if (!known_feature(f->name())) {
            diag_.error(f->start_loc(), "Argument '%s': Unknown feature '%s'") << ra.name() << f->name();
            return new (ctx_) error_expression(e);
         } else if (const target_ref* tr = as<target_ref>(f->value())) {
            if (tr->is_public()) {
               diag_.error(f->value()->start_loc(), "Argument '%s': Public target is not allowed here") << ra.name();
               return new (ctx_) error_expression(e);
            }
         }
         return e;
      } else {
         diag_.error(e->start_loc(), "Argument '%s': Expected feature or condition") << ra.name();
         return new (ctx_) error_expression(e);
      }
   };

   auto process_one_requirement = [&](const expression* e) -> const expression* {
      if (const condition_expr* ce = as<condition_expr>(e)) {
         const expression* condition = process_condition(ra, ce->condition());
         if (const list_of* l = as<list_of>(ce->result())) {
            expressions_t values{expressions_t::allocator_type{ctx_}};
            for (const expression* le : l->values())
               values.push_back(process_result_element(le));

            return new (ctx_) condition_expr(condition, new (ctx_) list_of(values));
         } else
            return new (ctx_) condition_expr(condition, process_result_element(ce->result()));
      } else
         return process_result_element(e);
   };

   if (const list_of* l = as<list_of>(arg)) {
      expressions_t values{expressions_t::allocator_type{ctx_}};
      for (const expression* le : l->values())
         values.push_back(process_one_requirement(le));

      return new (ctx_) usage_requirements{new (ctx_) list_of(values)};
   } else
      return new (ctx_) usage_requirements(process_one_requirement(arg));
}

const expression*
actions_impl::process_path_arg(const rule_argument& ra,
                               const expression* arg)
{
   if (const path* p = as<path>(arg)) {
      for (const expression* pe : p->elements()) {
         if (ast::as<id_expr>(pe))
            continue;
         else { // its wildcard list_of
            diag_.error(pe->start_loc(), "Wildcards not allowed here");
            return new (ctx_) error_expression(arg);
         }
      }

      return arg;
   }

   diag_.error(arg->start_loc(), "Path expected");
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_path_or_list_of_paths(const rule_argument& ra,
                                            const expression* arg)
{
   auto check_path = [&](const expression* e) -> const expression* {
      if (const path* p = as<path>(e)) {
         for (const expression* pe : p->elements()) {
            if (ast::as<id_expr>(pe))
               continue;
            else { // its wildcard list_of
               diag_.error(pe->start_loc(), "Wildcards not allowed here");
               return new (ctx_) error_expression(e);
            }
         }

         return e;
      } else {
         diag_.error(e->start_loc(), "Path expected");
         return new (ctx_) error_expression(e);
      }
   };

   if (const list_of* l = as<list_of>(arg)) {
      expressions_t elements{expressions_t::allocator_type{ctx_}};
      for (auto le : l->values())
         elements.push_back(check_path(le));

      return new (ctx_) list_of(elements);
   } else if (as<path>(arg))
      return check_path(arg);

   diag_.error(arg->start_loc(), "Argument '%s': Path or list of paths expected") << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_wcpath_arg(const rule_argument& ra,
                                 const expression* arg)
{
   if (as<path>(arg))
      return arg;

   diag_.error(arg->start_loc(), "Argument '%s': Path expected") << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_wcpath_or_list_of_wcpaths(const rule_argument& ra,
                                                const expression* arg)
{
   auto check_path = [&](const expression* e) -> const expression* {
      if (as<path>(e))
         return e;
      else {
         diag_.error(e->start_loc(), "Path expected");
         return new (ctx_) error_expression(e);
      }
   };

   if (const list_of* l = as<list_of>(arg)) {
      expressions_t elements{expressions_t::allocator_type{ctx_}};
      for (auto le : l->values())
         elements.push_back(check_path(le));

      return new (ctx_) list_of(elements);
   } else if (as<path>(arg))
      return check_path(arg);

   diag_.error(arg->start_loc(), "Argument '%s': Path or list of paths expected") << ra.name();
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
actions_impl::process_one_arg(const rule_argument& ra,
                              const expression* arg)
{
   if (!ra.is_optional() && is_a<empty_expr>(arg)) {
      diag_.error(as<empty_expr>(arg)->next_token().start_loc(), "Required argument '%s' expected before '%s'")
         << ra.name() 
         << as<empty_expr>(arg)->next_token();

      return new (ctx_) error_expression(arg->start_loc());
   }

   if (ra.is_optional() && is_a<empty_expr>(arg))
      return arg;

   const auto error_count = diag_.error_count();
   arg = ra.ast_transformer() ? ra.ast_transformer()(ctx_, diag_, arg) : arg;
   if (error_count != diag_.error_count())
      return arg;

   switch(ra.type()) {
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
         return process_path_arg(ra, arg);

      case rule_argument_type::path_or_list_of_paths:
         return process_path_or_list_of_paths(ra, arg);

      case rule_argument_type::wcpath:
         return process_wcpath_arg(ra, arg);

      case rule_argument_type::wcpath_or_list_of_wcpaths:
         return process_wcpath_or_list_of_wcpaths(ra, arg);

      case rule_argument_type::ast_expression:
         return arg;

      case rule_argument_type::feature_or_feature_set:
         return process_feature_of_feature_set_arg(ra, arg);

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
            diag_.error(arg_name.start_loc(), "%s '%s' does not have named argument '%s'")
               << (rule_decl.is_target() ? "Target" : "Rule") 
               << rule_name 
               << arg_name;

            result.push_back(new (ctx_) error_expression((**i).start_loc()));
         } else { // named argument founded
            if (used_named_args.find(&*r) != used_named_args.end()) {
               diag_.error(arg_name.start_loc(), "%s '%s': argument '%s' used more than once")
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
      diag_.error(rule_name.start_loc(), "%s '%s': not enough arguments")
         << (rule_decl.is_target() ? "Target" : "Rule") << rule_name;
   }

   if (ra == rule_decl.end() && rac != required_argument_used) {
      diag_.error(rule_name.start_loc(), "%s '%s': too many arguments")
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
            diag_.error(rule_name.start_loc(), "Rule 'project' MUST be the first statement in the file");
      } else
         first_rule_in_file_ = false;

      return new (ctx_) ast::rule_invocation(rule_name, process_arguments(rule_name, i->second, arguments));
   } else {
      diag_.error(rule_name.start_loc(), "Target or rule '%s' was not defined") << rule_name;
      return new (ctx_) ast::error_expression(rule_name.start_loc());
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

bool actions_impl::known_feature(const parscore::identifier& name) const
{
   if (features_.find(name.to_string()) != features_.end())
      return true;
   else
      return env_.known_feature(name);
}

}}

