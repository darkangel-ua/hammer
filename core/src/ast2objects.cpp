#include <hammer/core/ast2objects.h>
#include <hammer/ast/hamfile.h>
#include <hammer/core/rule_manager.h>
#include <hammer/ast/statement.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/rule_invocation.h>
#include <hammer/core/sources_decl.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_set.h>
#include <hammer/ast/sources.h>
#include <hammer/ast/path.h>
#include <hammer/ast/requirement_set.h>
#include <hammer/ast/usage_requirements.h>
#include <hammer/ast/feature_set.h>
#include <hammer/ast/feature.h>
#include <hammer/ast/list_of.h>
#include <hammer/ast/target_ref.h>
#include <hammer/ast/target_def.h>
#include <hammer/ast/condition.h>

namespace hammer {

static
feature*
ast2feature(invocation_context& ctx,
            const ast::feature& f)
{
   if (const ast::id_expr* id = ast::as<ast::id_expr>(f.value()))
      return ctx.current_project_.get_engine()->feature_registry().create_feature(f.name().to_string(), id->id().to_string());
   else
      throw std::runtime_error("Not implemented");
}

static
feature_set*
ast2feature_set(invocation_context& ctx,
                const ast::features_t& features)
{
   feature_set* result = ctx.current_project_.get_engine()->feature_registry().make_set();

   for (auto f : features)
      result->join(ast2feature(ctx, *f));

   return result;
}

static
feature_set*
ast2feature_set(invocation_context& ctx,
                const ast::list_of& features)
{
   feature_set* result = ctx.current_project_.get_engine()->feature_registry().make_set();

   for (auto le : features.values()) {
      if (const ast::feature* f = ast::as<ast::feature>(le))
         result->join(ast2feature(ctx, *f));
      else
         throw std::runtime_error("ast2feature_set: Unexpected AST node");
   }

   return result;
}

static
source_decl
handle_one_source(invocation_context& ctx,
                  const type_registry& tr,
                  const ast::expression* e)
{
   if (const ast::id_expr* v = ast::as<ast::id_expr>(e))
      return source_decl(v->id().to_string(), std::string(), tr.resolve_from_target_name(v->id().to_string()), nullptr);
   else if (const ast::path* v = ast::as<ast::path>(e))
      return source_decl(v->to_string(), std::string(), tr.resolve_from_target_name(v->to_string()), nullptr);
   else if (const ast::public_expr* pe = ast::as<ast::public_expr>(e)) {
      source_decl result = handle_one_source(ctx, tr, pe->value());
      result.set_public(true);
      return result;
   } else if (const ast::target_ref* v = ast::as<ast::target_ref>(e)) {
      feature_set* build_request = v->build_request().empty() ? nullptr : ast2feature_set(ctx, v->build_request());
      source_decl sd(v->target_path()->to_string(), v->target_name().valid() ? v->target_name().to_string() : std::string(), nullptr, build_request);
      sd.set_public(v->is_public());
      return sd;
   } else
      throw std::runtime_error("ast2sources_decl: Unexpected AST node");
}

static
std::unique_ptr<sources_decl>
ast2sources_decl(invocation_context& ctx,
                 const ast::sources& sources)
{
   std::unique_ptr<sources_decl> result(new sources_decl);
   const type_registry& tr = ctx.current_project_.get_engine()->get_type_registry();

   if (const ast::list_of* v = ast::as<ast::list_of>(sources.content())) {
      for (const ast::expression* e : v->values())
         result->push_back(handle_one_source(ctx, tr, e));
   } else
      result->push_back(handle_one_source(ctx, tr, sources.content()));

   return result;
}

static
std::unique_ptr<requirement_condition_op_base>
ast2requirement_condition_op(invocation_context& ctx,
                             const ast::expression* e)
{
   if (const ast::logical_or* lor = ast::as<ast::logical_or>(e)) {
      auto left = ast2requirement_condition_op(ctx, lor->left());
      auto right = ast2requirement_condition_op(ctx, lor->right());
      return std::unique_ptr<requirement_condition_op_base>(new requirement_condition_op_or(std::move(left), std::move(right)));
   } else if (const ast::logical_and* land = ast::as<ast::logical_and>(e)) {
      auto left = ast2requirement_condition_op(ctx, land->left());
      auto right = ast2requirement_condition_op(ctx, land->right());
      return std::unique_ptr<requirement_condition_op_base>(new requirement_condition_op_and(std::move(left), std::move(right)));
   } else if (const ast::feature* f = ast::as<ast::feature>(e))
     return std::unique_ptr<requirement_condition_op_base>(new requirement_condition_op_feature(ast2feature(ctx, *f)));
   else
      throw std::runtime_error("ast2requirement_condition_op: Unexpected AST node");
}

static
requirement_condition::result_t
ast2requirement_condition_result(invocation_context& ctx,
                                 const ast::expression* e)
{
   requirement_condition::result_t result;
   auto handle_one_feature = [&](const ast::expression* e) {
      if (const ast::feature* f = ast::as<ast::feature>(e))
         result.push_back({ast2feature(ctx, *f), false});
      else if (const ast::public_expr* pe = ast::as<ast::public_expr>(e)) {
         assert(ast::as<ast::feature>(pe->value()));
         const ast::feature* f = ast::as<ast::feature>(pe->value());
         result.push_back({ast2feature(ctx, *f), true});
      } else
         throw std::runtime_error("ast2requirement_condition_result: Unexpected AST node");
   };

   if (const ast::list_of* l = ast::as<ast::list_of>(e)) {
      for (auto le : l->values())
         handle_one_feature(le);
   } else
      handle_one_feature(e);

   return result;
}

static
std::auto_ptr<requirement_base>
ast2requirement_base(invocation_context& ctx,
                     const ast::expression* e,
                     const bool public_)
{
   if (const ast::feature* v = ast::as<ast::feature>(e))
      return std::auto_ptr<requirement_base>(new just_feature_requirement(ast2feature(ctx, *v)));
   else if (const ast::public_expr* pe = ast::as<ast::public_expr>(e)) {
      std::auto_ptr<requirement_base> rb = ast2requirement_base(ctx, pe->value(), true);
      rb->set_public(true);
      return rb;
   } else if (const ast::condition_expr* c = ast::as<ast::condition_expr>(e))
      return std::auto_ptr<requirement_base>(new requirement_condition(ast2requirement_condition_op(ctx, c->condition()), ast2requirement_condition_result(ctx, c->result()), public_));
   else
      throw std::runtime_error("ast2requirement_base: Unexpected AST node");
}

static
std::unique_ptr<usage_requirements_decl>
ast2usage_requirements_decl_impl(invocation_context& ctx,
                                 const ast::expression* requirements)
{
   std::unique_ptr<usage_requirements_decl> result(new usage_requirements_decl);

   if (const ast::list_of* l = ast::as<ast::list_of>(requirements)) {
      for (const ast::expression* e : l->values())
         result->add(ast2requirement_base(ctx, e, false));
   } else
      result->add(ast2requirement_base(ctx, requirements, false));

   return result;
}

static
std::unique_ptr<requirements_decl>
ast2requirements_decl(invocation_context& ctx,
                      const ast::requirement_set& requirements)
{
   return ast2usage_requirements_decl_impl(ctx, requirements.requirements());
}

static
std::unique_ptr<usage_requirements_decl>
ast2usage_requirements_decl(invocation_context& ctx,
                            const ast::usage_requirements& usage_requirements)
{
   return ast2usage_requirements_decl_impl(ctx, usage_requirements.requirements());
}

static
void ast2objects(invocation_context& ctx,
                 const ast::rule_invocation& ri)
{
   const rule_declaration& rd = ctx.rule_manager_.find(ri.name())->second;
   auto i_ri = ri.arguments().begin();
   rule_manager_arguments_t args;

   for (const rule_argument& ra : rd.arguments()) {
      if (i_ri == ri.arguments().end()) {
         args.push_back(rule_manager_arg_ptr());
         continue;
      }

      // named argument handling not implemented yet
      if (ast::is_a<ast::named_expr>(*i_ri))
         throw std::runtime_error("not implemented");

      if (ra.is_optional() && ast::as<ast::empty_expr>(*i_ri)) {
         rule_manager_arg_ptr arg;
         args.push_back(move(arg));
      } else {
         switch (ra.type()) {
            case rule_argument_type::invocation_context: {
               rule_manager_arg_ptr ctx_arg(new rule_manager_arg<invocation_context>(ctx));
               args.push_back(move(ctx_arg));
               continue;
            }

            case rule_argument_type::identifier: {
               const ast::id_expr* id = ast::as<ast::id_expr>(*i_ri);
               assert(id);
               rule_manager_arg_ptr id_arg(new rule_manager_arg<parscore::identifier>(new parscore::identifier(id->id())));
               args.push_back(move(id_arg));
               break;
            }

            case rule_argument_type::sources: {
               const ast::sources* sources = ast::as<ast::sources>(*i_ri);
               assert(sources);
               rule_manager_arg_ptr ctx_arg(new rule_manager_arg<sources_decl>(ast2sources_decl(ctx, *sources)));
               args.push_back(move(ctx_arg));
               break;
            }

            case rule_argument_type::requirement_set: {
               const ast::requirement_set* requirements = ast::as<ast::requirement_set>(*i_ri);
               assert(requirements);
               rule_manager_arg_ptr ctx_arg(new rule_manager_arg<requirements_decl>(ast2requirements_decl(ctx, *requirements)));
               args.push_back(move(ctx_arg));
               break;
            }

            case rule_argument_type::usage_requirements: {
               const ast::usage_requirements* usage_requirements = ast::as<ast::usage_requirements>(*i_ri);
               assert(usage_requirements);
               rule_manager_arg_ptr ctx_arg(new rule_manager_arg<usage_requirements_decl>(ast2usage_requirements_decl(ctx, *usage_requirements)));
               args.push_back(move(ctx_arg));
               break;
            }

            default:
               throw std::runtime_error("not implemented");
         }
      }

      ++i_ri;
   }

   rd.invoke(args);
}

static
void ast2objects(invocation_context& ctx,
                 const ast::target_def& td)
{
   ast2objects(ctx, *td.body());
}

void ast2objects(invocation_context& ctx,
                 const ast::hamfile& node)
{
   const rule_declaration& project_rd = ctx.rule_manager_.find(parscore::identifier("project"))->second;
   rule_manager_arguments_t args;

   rule_manager_arg_ptr ctx_arg(new rule_manager_arg<invocation_context>(ctx));
   args.push_back(move(ctx_arg));

   if (const ast::rule_invocation* p = node.get_project_def()) {
      throw std::runtime_error("not implemented");
   } else {
      parscore::identifier id("<unnamed>");
      rule_manager_arg_ptr id_arg(new rule_manager_arg<parscore::identifier>(id));
      rule_manager_arg_ptr requirements_arg, usage_requirements_arg;
      args.push_back(move(id_arg));
      args.push_back(move(requirements_arg));
      args.push_back(move(usage_requirements_arg));
      project_rd.invoke(args);
   }

   for (const ast::statement* stmt : node.get_statements()) {
      if (const ast::expression_statement* estmt = ast::as<ast::expression_statement>(stmt)) {
         if (const ast::rule_invocation* ri = ast::as<ast::rule_invocation>(estmt->content()))
            ast2objects(ctx, *ri);
         else
            throw std::runtime_error("ast2objects: Unexpected top level AST expression statement node");
      } else if (const ast::target_def* td = ast::as<ast::target_def>(stmt))
         ast2objects(ctx, *td);
       else
         throw std::runtime_error("not implemented");
   }
}

}
