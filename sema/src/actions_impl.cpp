#include <set>
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
#include <hammer/ast/struct_expr.h>
#include <hammer/core/diagnostic.h>
#include <hammer/sema/actions_impl.h>

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
                  diag_.error(feature_name.start_loc(), "Feature '%s' already defined") << feature_name;
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
actions_impl::process_identifier_arg(const char* prefix,
                                     const rule_argument& ra,
                                     const expression* arg)
{
   if (const id_expr* expr = as<id_expr>(arg))
      return expr;

   diag_.error(arg->start_loc(), "%s '%s': Identifier expected") << prefix << ra.name();
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
actions_impl::process_feature_set_arg(const char* prefix,
                                      const rule_argument& ra,
                                      const expression* arg)
{
   auto process_one = [&](const expression* e) -> const expression* {
      if (const feature* f = as<feature>(e)) {
         if (!known_feature(f->name())) {
            diag_.error(f->start_loc(), "%s '%s': Unknown feature '%s'") << prefix << ra.name() << f->name();
            return new (ctx_) error_expression(e);
         } else
            return e;
      } else {
         diag_.error(e->start_loc(), "%s '%s': Feature set elements are simple features") << prefix << ra.name();
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
actions_impl::process_feature_arg(const char* prefix,
                                  const rule_argument& ra,
                                  const expression* arg)
{
   if (const feature* f = ast::as<feature>(arg)) {
      if (!known_feature(f->name())) {
         diag_.error(arg->start_loc(), "%s '%s': Unknown feature '%s'") << prefix << ra.name() << f->name();
         return new (ctx_) error_expression(arg);
      } else
         return arg;
   }

   diag_.error(arg->start_loc(), "%s '%s': Feature expected") << prefix << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::wrap_public(const expression* e,
                          const public_expr* pe) {
   if (pe)
      return new (ctx_) public_expr{pe->tag(), e};
   else
      return e;
}

const expression*
actions_impl::process_sources_arg(const rule_argument& ra,
                                  const expression* arg)
{
   auto process_one_source = [&](const expression* e) -> const expression* {
      if (as<id_expr>(e))
         return e;
      else if (const path* p = as<path>(e)) {
         if (p->has_wildcard()) {
            diag_.error(e->start_loc(), "Argument '%s': Path with wildcards not allowed in sources") << ra.name();
            return new (ctx_) error_expression(p);
         } else
            return e;
      } else if (const target_ref* tr = as<target_ref>(e)) {
         if (tr->target_path()->has_wildcard()) {
            diag_.error(e->start_loc(), "Argument '%s': Wildcards not allowed in target reference") << ra.name();
            return new (ctx_) error_expression(e);
         } else
            return e;
      } else if (const rule_invocation* ri = as<rule_invocation>(e)) {
         // cannot fail because rule has been checked previously
         const rule_declaration& ri_rd = rule_manager_.find(ri->name())->second;
         const rule_argument& rdr = ri_rd.result();
         // we can deal only with return types that can be converted to sources
         if (*rdr.type().as_simple() == rule_argument_type::identifier ||
             *rdr.type().as_simple() == rule_argument_type::sources ||
             *rdr.type().as_simple() == rule_argument_type::path ||
             *rdr.type().as_simple() == rule_argument_type::target_ref)
         {
            return ri;
         }

         diag_.error(ri->start_loc(), "Argument '%s': Can't use result of rule '%s' as source") << ra.name() << ri->name();
         return new (ctx_) error_expression(ri);
      } else {
         diag_.error(e->start_loc(), "Argument '%s': Expected id, path, target reference or rule invocation") << ra.name();
         return new (ctx_) error_expression(e);
      }
   };

   if (const list_of* l = as<list_of>(arg)) {
      expressions_t values{expressions_t::allocator_type{ctx_}};
      for (const expression* le : l->values())
         values.push_back(process_one_source(le));

      return new (ctx_) sources{new (ctx_) list_of(values)};
   } else
      return new (ctx_) sources(process_one_source(arg));
}

const expression*
actions_impl::process_requirements_decl_arg(const char* prefix,
                                            const rule_argument& ra,
                                            const expression* arg)
{
   auto process_result_element = [&](const expression* e,
                                     const bool public_requirement) -> const expression* {
      const public_expr* pe = as<public_expr>(e);
      if (pe) {
         if (public_requirement) {
            diag_.error(e->start_loc(), "%s '%s': Public feature is not allowed here") << prefix << ra.name();
            return new (ctx_) error_expression(e);
         } else
            e = pe->value();
      }

      if (const feature* f = as<feature>(e)) {
         if (!known_feature(f->name())) {
            diag_.error(f->start_loc(), "%s '%s': Unknown feature '%s'") << prefix << ra.name() << f->name();
            return new (ctx_) error_expression(e);
         } else if (const target_ref* tr = as<target_ref>(f->value())) {
            if (tr->is_public() && public_requirement) {
               diag_.error(f->value()->start_loc(), "%s '%s': Public target is not allowed here") << prefix << ra.name();
               return new (ctx_) error_expression(e);
            }
         }
         return wrap_public(e, pe);
      } else {
         diag_.error(e->start_loc(), "%s '%s': Expected feature or condition") << prefix << ra.name();
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
actions_impl::process_path_arg(const char* prefix,
                               const rule_argument& ra,
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

   diag_.error(arg->start_loc(), "%s '%s': Path expected") << prefix << ra.name();
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
actions_impl::process_wcpath_arg(const char* prefix,
                                 const rule_argument& ra,
                                 const expression* arg)
{
   if (as<path>(arg))
      return arg;

   diag_.error(arg->start_loc(), "%s '%s': Path expected") << prefix << ra.name();
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
actions_impl::process_feature_of_feature_set_arg(const char* prefix,
                                                 const rule_argument& ra,
                                                 const expression* arg)
{
   if (as<ast::feature>(arg))
      return process_feature_arg(prefix, ra, arg);
   else if (as<ast::list_of>(arg))
      return process_feature_set_arg(prefix, ra, arg);

   diag_.error(arg->start_loc(), "%s '%s': expected feature or feature list") << prefix << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_target_ref_arg(const char* prefix,
                                     const rule_argument& ra,
                                     const expression* arg)
{
   if (const target_ref* tr = as<target_ref>(arg)) {
      if (tr->target_path()->has_wildcard()) {
         diag_.error(tr->start_loc(), "%s '%s': Wildcards not allowed in target reference") << prefix << ra.name();
         return new (ctx_) error_expression(tr);
      } else
         return arg;
   }

   diag_.error(arg->start_loc(), "%s '%s': Expected target reference") << prefix << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_target_ref_mask_arg(const char* prefix,
                                          const rule_argument& ra,
                                          const expression* arg)
{
   if (as<id_expr>(arg))
      return arg;
   else if (const path* p = as<path>(arg)) {
      if (p->root_name().valid()) {
         diag_.error(p->start_loc(), "%s '%s': Absolute path can't be used as target reference mask") << prefix << ra.name();
         return new (ctx_) error_expression(p);
      }
      return arg;
   } else if (const target_ref* tr = as<target_ref>(arg)) {
      if (tr->public_tag().valid()) {
         diag_.error(tr->public_tag(), "%s '%s': Target reference mask can't be public") << prefix << ra.name();
         return new (ctx_) error_expression(tr);
      }
      if (tr->has_target_name()) {
         diag_.error(tr->target_name().start_loc(), "%s '%s': Target name is not allowed in target reference mask") << prefix << ra.name();
         return new (ctx_) error_expression(tr);
      }
      if (!tr->build_request().empty()) {
         diag_.error(tr->build_request().front()->start_loc(), "%s '%s': Build request is not allowed in target reference mask") << prefix << ra.name();
         return new (ctx_) error_expression(tr);
      } else
         return arg;
   }

   diag_.error(arg->start_loc(), "%s '%s': Target reference mask expected") << prefix << ra.name();
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_struct_arg(const char* prefix,
                                 const rule_argument& ra,
                                 const rule_argument_struct_desc& rasd,
                                 const expression* arg) {
   if (const struct_expr* s = as<struct_expr>(arg))
      return new (ctx_) struct_expr(s->start_brace_,
                                    process_arguments_impl("Field", "Struct", "field", rasd.typename_.c_str(), s->start_brace_,
                                                           rasd.fields_->begin(), rasd.fields_->end(),
                                                           s->fields_.begin(), s->fields_.end()));

   diag_.error(arg->start_loc(), "%s '%s': Expected struct of type '%s'") << prefix << ra.name() << rasd.typename_;
   return new (ctx_) error_expression(arg);
}

const expression*
actions_impl::process_one_or_list_arg(const char* prefix,
                                      const rule_argument& ra,
                                      const rule_argument_type_desc& nested_type,
                                      const expression* arg) {
   auto process_one = [&] (const expression* e) -> const expression* {
      if (auto type = nested_type.as_simple()) {
         switch (*type) {
            case rule_argument_type::identifier:
               return process_identifier_arg(prefix, ra, e);
            case rule_argument_type::feature:
               return process_feature_arg(prefix, ra, e);
            case rule_argument_type::path:
               return process_path_arg(prefix, ra, e);
            case rule_argument_type::wcpath:
               return process_wcpath_arg(prefix, ra, e);
            case rule_argument_type::target_ref:
               return process_target_ref_arg(prefix, ra, e);
            case rule_argument_type::target_ref_mask:
               return process_target_ref_mask_arg(prefix, ra, e);
            default:
               break;
         }
      } else if (auto type = nested_type.as_struct())
         return process_struct_arg(prefix, ra, *type, e);
      else if (auto type = nested_type.as_variant())
         throw std::runtime_error("[actions_impl::process_one_or_list_arg]: variants not implemented");

      assert(false && "[actions_impl::process_one_or_list_arg]: Unhandled argument type!");
      abort();
   };

   if (const list_of* l = as<list_of>(arg)) {
      expressions_t values{expressions_t::allocator_type{ctx_}};
      for (const expression* le : l->values())
         values.push_back(process_one(le));

      return new (ctx_) list_of(values);
   } else
      return process_one(arg);
}

const expression*
actions_impl::process_one_arg(const char* prefix,
                              const rule_argument& ra,
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

   if (auto simple_type_ptr = ra.type().as_simple()) {
      switch(*simple_type_ptr) {
         case rule_argument_type::identifier:
            return process_identifier_arg(prefix, ra, arg);

         case rule_argument_type::identifier_or_list_of_identifiers:
            return process_identifier_or_list_of_identifiers_arg(ra, arg);

         case rule_argument_type::feature:
            return process_feature_arg(prefix, ra, arg);

         case rule_argument_type::feature_set:
            return process_feature_set_arg(prefix, ra, arg);

         case rule_argument_type::sources:
            return process_sources_arg(ra, arg);

         case rule_argument_type::requirement_set:
            return process_requirements_decl_arg(prefix, ra, arg);

         case rule_argument_type::usage_requirements:
            return process_usage_requirements_arg(ra, arg);

         case rule_argument_type::path:
            return process_path_arg(prefix, ra, arg);

         case rule_argument_type::path_or_list_of_paths:
            return process_path_or_list_of_paths(ra, arg);

         case rule_argument_type::wcpath:
            return process_wcpath_arg(prefix, ra, arg);

         case rule_argument_type::wcpath_or_list_of_wcpaths:
            return process_wcpath_or_list_of_wcpaths(ra, arg);

         case rule_argument_type::ast_expression:
            return arg;

         case rule_argument_type::feature_or_feature_set:
            return process_feature_of_feature_set_arg(prefix, ra, arg);

         case rule_argument_type::target_ref:
            return process_target_ref_arg(prefix, ra, arg);

         case rule_argument_type::target_ref_mask:
            return process_target_ref_mask_arg(prefix, ra, arg);

         default:
            assert(false && "Unknown argument type");
            abort();
      }
   } else if (auto list_type_ptr = ra.type().as_list())
      return process_one_or_list_arg(prefix, ra, *list_type_ptr->nested_type_, arg);
   else if (auto struct_type_ptr = ra.type().as_struct()) {
      return process_struct_arg(prefix, ra, *struct_type_ptr, arg);
   } else {
      assert(false && "Unhandled rule argument type");
      abort();
   }
}

expressions_t
actions_impl::process_arguments_impl(const char* prefix,
                                     const char* kind,
                                     const char* argument_or_field,
                                     const char* rule_or_struct_name,
                                     parscore::source_location rule_or_struct_name_loc,
                                     rule_declaration::const_iterator args_decl_first,
                                     rule_declaration::const_iterator args_decl_last,
                                     ast::expressions_t::const_iterator arguments_first,
                                     ast::expressions_t::const_iterator arguments_last)
{
   using used_named_args_t = std::set<const rule_argument*>;

   expressions_t result{expressions_t::allocator_type{ctx_}};
   used_named_args_t used_named_args;

   bool only_named = false;
   bool has_error = false;
   auto ra = args_decl_first;
   int required_argument_used = 0;
   for (auto i = arguments_first, last = arguments_last; i != last; ++i) {
      if (const named_expr* ne = as<named_expr>(*i)) {
         const identifier& arg_name = as<named_expr>(**i).name();
         auto r = std::find_if(args_decl_first, args_decl_last, [&](const rule_argument& ra) {
            return ra.name() == arg_name;
         });

         if (r == args_decl_last) {
            has_error = true;
            diag_.error(arg_name.start_loc(), "%s '%s' does not have named %s '%s'")
               << kind
               << rule_or_struct_name
               << argument_or_field
               << arg_name;

            result.push_back(new (ctx_) error_expression((**i).start_loc()));
         } else { // named argument founded
            if (used_named_args.find(&*r) != used_named_args.end()) {
               has_error = true;
               diag_.error(arg_name.start_loc(), "%s '%s': %s '%s' used more than once")
                  << kind
                  << rule_or_struct_name
                  << argument_or_field
                  << arg_name;

               result.push_back(new (ctx_) error_expression((**i).start_loc()));
            } else {
               used_named_args.insert(&*r);
               if (!r->is_optional())
                  ++required_argument_used;

               const named_expr* new_ne = new (ctx_) named_expr(ne->name(), process_one_arg(prefix, *r, ne->value()));
               result.push_back(new_ne);
            }
         }

         only_named = true;
      } else { // unnamed argument
         // too many arguments
         if (ra == args_decl_last) {
            result.push_back(*i);
            ++required_argument_used;
            continue;
         }

         if (is_a<error_expression>(**i)) {
            if (!ra->is_optional())
               ++required_argument_used;

            result.push_back(*i);
         } else if (only_named) {
            diag_.error((**i).start_loc(), "Named %s expected") << argument_or_field;
            return result;
         } else {
            if (!ra->is_optional())
               ++required_argument_used;

            result.push_back(process_one_arg(prefix, *ra, *i));
         }
      }

      if (ra != args_decl_last)
         ++ra;
   }

   int rac = std::count_if(args_decl_first, args_decl_last, [](const rule_argument& ra) {
      return !ra.is_optional();
   });

   if (!has_error) {
      if (rac > required_argument_used) {
         diag_.error(rule_or_struct_name_loc, "%s '%s': not enough %ss")
            << kind << rule_or_struct_name << argument_or_field;
      } else if (ra == args_decl_last && rac != required_argument_used) {
         diag_.error(rule_or_struct_name_loc, "%s '%s': too many %ss")
            << kind << rule_or_struct_name << argument_or_field;
      }
   }

   return result;
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

               const named_expr* new_ne = new (ctx_) named_expr(ne->name(), process_one_arg("Argument", *r, ne->value()));
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

            result.push_back(process_one_arg("Argument", *ra, *i));
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

const expression*
actions_impl::on_struct(source_location start_brace,
                        const expressions_t& fields)
{
   return new (ctx_) ast::struct_expr(start_brace, fields);
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

