#include <hammer/core/ast2objects.h>
#include <hammer/ast/hamfile.h>
#include <hammer/core/rule_manager.h>
#include <hammer/ast/statement.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/rule_invocation.h>
#include <hammer/core/sources_decl.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/ast/sources.h>
#include <hammer/ast/path.h>
#include <hammer/ast/requirement_set.h>
#include <hammer/ast/usage_requirements.h>
#include <hammer/ast/feature_set.h>

using std::move;

namespace hammer {

static
std::unique_ptr<sources_decl>
ast2sources_decl(invocation_context& ctx,
                 const ast::sources& sources)
{
   std::unique_ptr<sources_decl> result(new sources_decl);

   if (const ast::id_expr* v = ast::as<ast::id_expr>(sources.content())) {
      const type_registry& tr = ctx.current_project_.get_engine()->get_type_registry();
      result->push_back(v->id().to_string(), tr);
   } else if (const ast::path* v = ast::as<ast::path>(sources.content())) {
      const type_registry& tr = ctx.current_project_.get_engine()->get_type_registry();
      result->push_back(v->to_string(), tr);
   } else
      throw std::runtime_error("Not implemented");

   return result;
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
               throw std::runtime_error("not implemented");

               break;
            }

            case rule_argument_type::usage_requirements: {
               const ast::usage_requirements* usage_requirements = ast::as<ast::usage_requirements>(*i_ri);
               assert(usage_requirements);
               throw std::runtime_error("not implemented");

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
      const ast::expression_statement* estmt = ast::as<ast::expression_statement>(stmt);
      assert(estmt);

      if (const ast::rule_invocation* ri = ast::as<ast::rule_invocation>(estmt->content()))
         ast2objects(ctx, *ri);
      else
         throw std::runtime_error("not implemented");
   }
}

}
