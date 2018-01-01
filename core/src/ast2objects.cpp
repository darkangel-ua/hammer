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
#include <hammer/core/feature.h>
#include <hammer/core/diagnostic.h>
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
#include <boost/make_unique.hpp>

namespace hammer {

static
source_decl
handle_one_source(invocation_context& ctx,
                  const type_registry& tr,
                  const ast::expression* e);
static
rule_manager_arg_ptr
process_rule_invocation(invocation_context& ctx,
                        const ast::rule_invocation& ri);

static
feature*
ast2feature(invocation_context& ctx,
            const ast::feature& f)
{

   feature_registry* fr = &ctx.current_project_.local_feature_registry();
   const feature_def* fdef = fr->find_def(f.name().to_string().c_str());
   if (!fdef) {
      fr = &ctx.current_project_.get_engine()->feature_registry();
      fdef = fr->find_def(f.name().to_string().c_str());
   }

   if (fdef && fdef->attributes().dependency) {
      feature* result = fr->create_feature(f.name().to_string(), {});
      result->set_dependency_data(handle_one_source(ctx, ctx.current_project_.get_engine()->get_type_registry(), f.value()), nullptr);
      return result;
   } else if (const ast::id_expr* id = ast::as<ast::id_expr>(f.value()))
      return fr->create_feature(f.name().to_string(), id->id().to_string());
   else if (ast::is_a<ast::target_ref>(f.value())) {
      feature* result = fr->create_feature(f.name().to_string(), {});
      result->set_dependency_data(handle_one_source(ctx, ctx.current_project_.get_engine()->get_type_registry(), f.value()), nullptr);
      return result;
   } else if (const ast::path* p = ast::as<ast::path>(f.value())) {
      feature* result = fr->create_feature(f.name().to_string(), p->to_string());
      return result;
   } else
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
feature_set*
ast2feature_set(invocation_context& ctx,
                const ast::feature_set& fs)
{
   if (const ast::feature* f = ast::as<ast::feature>(fs.values())) {
      feature_set* result = ctx.current_project_.get_engine()->feature_registry().make_set();
      result->join(ast2feature(ctx, *f));
      return result;
   } else if (const ast::list_of* l = ast::as<ast::list_of>(fs.values()))
      return ast2feature_set(ctx, *l);
   else
      throw std::runtime_error("ast2feature_set: Unexpected AST node");
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
void handle_one_source(invocation_context& ctx,
                       const type_registry& tr,
                       const ast::expression* e,
                       sources_decl& result)
{
   if (const ast::rule_invocation* ri = ast::as<ast::rule_invocation>(e)) {
      auto invocation_result = process_rule_invocation(ctx, *ri);
      assert(invocation_result);
      const rule_declaration& rd = ctx.rule_manager_.find(ri->name())->second;
      if (rd.result().type() == rule_argument_type::sources);
      switch (rd.result().type()) {
         case rule_argument_type::sources: {
            const sources_decl& sources = *static_cast<const sources_decl*>(invocation_result->value());
            result.insert(sources);
            break;
         }

         case rule_argument_type::identifier: {
            const parscore::identifier& id = *static_cast<const parscore::identifier*>(invocation_result->value());
            result.push_back(source_decl(id.to_string(), std::string(), tr.resolve_from_target_name(id.to_string()), nullptr));
            break;
         }

         case rule_argument_type::path: {
            const location_t& p = *static_cast<const location_t*>(invocation_result->value());
            result.push_back(source_decl(p.string(), std::string(), tr.resolve_from_target_name(p.string()), nullptr));
            break;
         }

         case rule_argument_type::target_ref: {
            const source_decl& source = *static_cast<const source_decl*>(invocation_result->value());
            result.push_back(source);
            break;
         }

         default:
            throw std::runtime_error("ast2sources_decl: Unexpected rule invocation result");
      }
   } else
      result.push_back(handle_one_source(ctx, tr, e));
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
         handle_one_source(ctx, tr, e, *result);
   } else
      handle_one_source(ctx, tr, sources.content(), *result);

   return result;
}

static
std::unique_ptr<id_or_list_of_ids_t>
ast2identifier_or_list_of_identifiers(invocation_context& ctx,
                                      const ast::expression* e)
{
   auto result = boost::make_unique<id_or_list_of_ids_t>();

   if (const ast::id_expr* id = ast::as<ast::id_expr>(e))
      result->push_back(id->id());
   else {
      const ast::list_of* l = ast::as<ast::list_of>(e);
      for (const ast::expression* le : l->values())
         result->push_back(ast::as<ast::id_expr>(le)->id());
   }

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
std::unique_ptr<location_t>
ast2path(invocation_context& ctx,
         const ast::path& path)
{
   return std::unique_ptr<location_t>(new location_t(path.to_string()));
}

static
std::unique_ptr<path_or_list_of_paths_t>
ast2path_or_list_of_paths(invocation_context& ctx,
                          const ast::expression& e)
{
   if (const ast::path* p = ast::as<ast::path>(&e))
      return boost::make_unique<path_or_list_of_paths_t>(*ast2path(ctx, *p));
   else if (const ast::list_of* l = ast::as<ast::list_of>(&e)) {
      std::vector<location_t> paths;
      for (const ast::expression* le : l->values())
         paths.push_back(*ast2path(ctx, *ast::as<ast::path>(le)));

      return boost::make_unique<path_or_list_of_paths_t>(std::move(paths));
   } else
      throw std::runtime_error("ast2feature_or_feature_set: Unexpected AST node");
}

static
const ast::expression*
find_named_argument(ast::expressions_t::const_iterator arg_first,
                    ast::expressions_t::const_iterator arg_last,
                    const parscore::identifier& name)
{
   for (; arg_first != arg_last; ++arg_first) {
      const ast::named_expr* ne = ast::as<ast::named_expr>(*arg_first);
      if (ne->name() == name)
         return ne->value();
   }

   return nullptr;
}

static
void handle_one_arg(invocation_context& ctx,
                    rule_manager_arguments_t& args,
                    const rule_argument& ra,
                    const ast::expression* e)
{
   if (ra.is_optional() && ast::as<ast::empty_expr>(e)) {
      rule_manager_arg_ptr arg;
      args.push_back(move(arg));
   } else {
      switch (ra.type()) {
         case rule_argument_type::identifier: {
            const ast::id_expr* id = ast::as<ast::id_expr>(e);
            assert(id);
            rule_manager_arg_ptr arg(new rule_manager_arg<parscore::identifier>(new parscore::identifier(id->id())));
            args.push_back(move(arg));
            break;
         }

         case rule_argument_type::identifier_or_list_of_identifiers: {
            rule_manager_arg_ptr arg(new rule_manager_arg<id_or_list_of_ids_t>(ast2identifier_or_list_of_identifiers(ctx, e)));
            args.push_back(move(arg));
            break;
         }

         case rule_argument_type::sources: {
            const ast::sources* sources = ast::as<ast::sources>(e);
            assert(sources);
            rule_manager_arg_ptr arg(new rule_manager_arg<sources_decl>(ast2sources_decl(ctx, *sources)));
            args.push_back(move(arg));
            break;
         }

         case rule_argument_type::requirement_set: {
            const ast::requirement_set* requirements = ast::as<ast::requirement_set>(e);
            assert(requirements);
            rule_manager_arg_ptr arg(new rule_manager_arg<requirements_decl>(ast2requirements_decl(ctx, *requirements)));
            args.push_back(move(arg));
            break;
         }

         case rule_argument_type::usage_requirements: {
            const ast::usage_requirements* usage_requirements = ast::as<ast::usage_requirements>(e);
            assert(usage_requirements);
            rule_manager_arg_ptr arg(new rule_manager_arg<usage_requirements_decl>(ast2usage_requirements_decl(ctx, *usage_requirements)));
            args.push_back(move(arg));
            break;
         }

         case rule_argument_type::ast_expression: {
            rule_manager_arg_ptr arg(new rule_manager_arg<ast::expression>(*e));
            args.push_back(move(arg));
            break;
         }

         case rule_argument_type::path: {
            const ast::path* path = ast::as<ast::path>(e);
            assert(path);
            rule_manager_arg_ptr arg(new rule_manager_arg<location_t>(ast2path(ctx, *path)));
            args.push_back(move(arg));
            break;
         }

         case rule_argument_type::path_or_list_of_paths: {
            rule_manager_arg_ptr arg(new rule_manager_arg<path_or_list_of_paths_t>(ast2path_or_list_of_paths(ctx, *e)));
            args.push_back(move(arg));
            break;
         }

         case rule_argument_type::feature: {
            const ast::feature* f = ast::as<ast::feature>(e);
            assert(f);
            rule_manager_arg_ptr arg(new rule_manager_arg<feature>(*ast2feature(ctx, *f)));
            args.push_back(move(arg));
            break;
         }

         case rule_argument_type::feature_set: {
            const ast::feature_set* fs = ast::as<ast::feature_set>(e);
            assert(fs);
            rule_manager_arg_ptr arg(new rule_manager_arg<feature_set>(*ast2feature_set(ctx, *fs)));
            args.push_back(move(arg));
            break;
         }

         case rule_argument_type::feature_or_feature_set: {
            rule_manager_arg_ptr arg(new rule_manager_arg<feature_or_feature_set_t>(ast2feature_or_feature_set(ctx, *e)));
            args.push_back(move(arg));
            break;
         }

         default:
            throw std::runtime_error("not implemented");
      }
   }
}

static
void rule_invocation_handle_named_args(invocation_context& ctx,
                                       rule_manager_arguments_t& args,
                                       rule_declaration::const_iterator rd_first,
                                       rule_declaration::const_iterator rd_last,
                                       ast::expressions_t::const_iterator arg_first,
                                       ast::expressions_t::const_iterator arg_last)
{
   for (; rd_first != rd_last; ++rd_first) {
      const ast::expression* e = find_named_argument(arg_first, arg_last, rd_first->name());
      if (e)
         handle_one_arg(ctx, args, *rd_first, e);
      else
         args.push_back(rule_manager_arg_ptr());
   }
}

static
rule_manager_arg_ptr
rule_invocation_impl(invocation_context& ctx,
                     rule_manager_arguments_t& args,
                     const ast::rule_invocation& ri)
{
   assert(ctx.rule_manager_.find(ri.name()) != ctx.rule_manager_.end());
   const rule_declaration& rd = ctx.rule_manager_.find(ri.name())->second;
   auto i_ri = ri.arguments().begin();
   // skipping first argument because it will always be context
   for ( auto first = rd.begin() + 1, last = rd.end(); first != last; ++first) {
      if (i_ri == ri.arguments().end()) {
         args.push_back(rule_manager_arg_ptr());
         continue;
      }

      if (ast::is_a<ast::named_expr>(*i_ri)) {
         rule_invocation_handle_named_args(ctx, args, first, last, i_ri, ri.arguments().end());
         break;
      }

      handle_one_arg(ctx, args, *first, *i_ri);
      ++i_ri;
   }

   return rd.invoke(args);
}

static
rule_manager_arg_ptr
process_rule_invocation(invocation_context& ctx,
                        const ast::rule_invocation& ri)
{
   rule_manager_arguments_t args;
   rule_manager_arg_ptr ctx_arg(new rule_manager_arg<invocation_context>(ctx));
   args.push_back(move(ctx_arg));

   return rule_invocation_impl(ctx, args, ri);
}

static
rule_manager_arg_ptr
process_target_rule_invocation(invocation_context& ctx,
                               const ast::target_def& td)
{
   rule_manager_arguments_t args;
   target_invocation_context tctx = { ctx.current_project_, ctx.diag_, ctx.rule_manager_, td.local_tag().valid(), td.explicit_tag().valid() };
   rule_manager_arg_ptr ctx_arg(new rule_manager_arg<target_invocation_context>(tctx));
   args.push_back(move(ctx_arg));

   return rule_invocation_impl(ctx, args, *td.body());
}

void ast2objects(invocation_context& ctx,
                 const ast::hamfile& node)
{
   const rule_declaration& project_rd = ctx.rule_manager_.find(parscore::identifier("project"))->second;
   rule_manager_arguments_t args;

   rule_manager_arg_ptr ctx_arg(new rule_manager_arg<invocation_context>(ctx));
   args.push_back(move(ctx_arg));

   if (const ast::rule_invocation* p = node.get_project_def())
      process_rule_invocation(ctx, *p);
   else {
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
            process_rule_invocation(ctx, *ri);
         else
            throw std::runtime_error("ast2objects: Unexpected top level AST expression statement node");
      } else if (const ast::target_def* td = ast::as<ast::target_def>(stmt))
         process_target_rule_invocation(ctx, *td);
       else
         throw std::runtime_error("not implemented");
   }
}

std::unique_ptr<feature_or_feature_set_t>
ast2feature_or_feature_set(invocation_context& ctx,
                           const ast::expression& e)
{
   if (const ast::feature* f = ast::as<ast::feature>(&e))
      return boost::make_unique<feature_or_feature_set_t>(ast2feature(ctx, *f));
   else if (const ast::feature_set* fs = ast::as<ast::feature_set>(&e))
      return boost::make_unique<feature_or_feature_set_t>(ast2feature_set(ctx, *fs));
   else
      throw std::runtime_error("ast2feature_or_feature_set: Unexpected AST node");
}

}
