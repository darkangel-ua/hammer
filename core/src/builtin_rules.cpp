#include "builtin_rules.h"
#include <hammer/core/rule_manager.h>
#include <hammer/core/sources_decl.h>
#include <hammer/core/requirements_decl.h>
#include <hammer/core/typed_meta_target.h>
#include <hammer/core/types.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/alias_meta_target.h>
#include <hammer/core/lib_meta_target.h>
#include <hammer/ast/target_ref.h>
#include <hammer/ast/path.h>
#include <hammer/ast/list_of.h>
#include <hammer/ast/casts.h>
#include <hammer/core/diagnostic.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/ast2objects.h>
#include <boost/variant/get.hpp>

using namespace std;

namespace hammer {

template<>
struct rule_argument_type_info<sources_decl> { static const rule_argument_type ast_type = rule_argument_type::sources; };

template<>
struct rule_argument_type_info<requirements_decl> { static const rule_argument_type ast_type = rule_argument_type::requirement_set; };

template<>
struct rule_argument_type_info<usage_requirements_decl> { static const rule_argument_type ast_type = rule_argument_type::usage_requirements; };

template<>
struct rule_argument_type_info<feature_set> { static const rule_argument_type ast_type = rule_argument_type::feature_set; };

template<>
struct rule_argument_type_info<location_t> { static const rule_argument_type ast_type = rule_argument_type::path; };

template<>
struct rule_argument_type_info<feature> { static const rule_argument_type ast_type = rule_argument_type::feature; };

template<>
struct rule_argument_type_info<feature_or_feature_set_t> { static const rule_argument_type ast_type = rule_argument_type::feature_or_feature_set; };

template<>
struct rule_argument_type_info<id_or_list_of_ids_t> { static const rule_argument_type ast_type = rule_argument_type::feature_or_feature_set; };

}

namespace hammer { namespace details {

static
void project_rule(invocation_context& ctx,
                  const parscore::identifier& id,
                  const requirements_decl* requirements,
                  const usage_requirements_decl* usage_requirements)
{
   ctx.current_project_.name(id.to_string());
   ctx.current_project_.requirements(requirements ? *requirements : requirements_decl());
   ctx.current_project_.usage_requirements(usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl());
}

static
void exe_rule(target_invocation_context& ctx,
              const parscore::identifier& id,
              const sources_decl& sources,
              const requirements_decl* requirements,
              const feature_set* default_build,
              const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new typed_meta_target(&ctx.current_project_,
                                                        id.to_string(),
                                                        requirements ? *requirements : requirements_decl(),
                                                        usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl(),
                                                        ctx.current_project_.get_engine()->get_type_registry().get(types::EXE)));
   mt->sources(sources);
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(mt);
}

static
void alias_rule(target_invocation_context& ctx,
                const parscore::identifier& name,
                const sources_decl& sources,
                const requirements_decl* requirements,
                const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new alias_meta_target(&ctx.current_project_, name.to_string(),
                                                        sources,
                                                        requirements ? *requirements : requirements_decl(),
                                                        usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(mt);
}

static
void lib_rule(target_invocation_context& ctx,
              const parscore::identifier& id,
              const sources_decl& sources,
              const requirements_decl* requirements,
              const feature_set* default_build,
              const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new lib_meta_target(&ctx.current_project_,
                                                      id.to_string(),
                                                      requirements ? *requirements : requirements_decl(),
                                                      usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   mt->sources(sources);
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(mt);
}

static
bool has_dots(const location_t& l) {
   for (auto e : l)
      if (e == "..")
         return true;

   return false;
}

static
void use_project_rule(invocation_context& ctx,
                      const ast::expression& alias,
                      const location_t& location,
                      feature_set* props)
{
   std::string project_alias;

   if (const ast::id_expr* id = ast::as<ast::id_expr>(&alias)) {
      project_alias = id->id().to_string();
   } else if (const ast::path* p = ast::as<ast::path>(&alias)) {
      std::string s_path = p->to_string();
      if (p->root().valid() ||
          s_path.find_first_of("?*") != std::string::npos ||
          has_dots(location))
      {
         // FIXME: How to inform user about correct location?
         ctx.diag_.error({}, "Argument 'location': Expected non-root path without wildcards and ..");
         throw std::runtime_error("Sematic error");
      }
   } else if (const ast::target_ref* tr = ast::as<ast::target_ref>(&alias)) {
      if (tr->is_public() || !tr->build_request().empty() || tr->target_name().valid()) {
         ctx.diag_.error(tr->start_loc(), "Argument 'alias': Expected non-public target reference expression without target name and build request");
         throw std::runtime_error("Sematic error");
      }

      if (tr->target_path()->root().valid() && ctx.current_project_.is_root()) {
         ctx.diag_.error(alias.start_loc(), "Argument 'alias': Global aliases can be declared only in homroot file");
         throw std::runtime_error("Sematic error");
      }

      project_alias = tr->target_path()->to_string();
   } else {
      ctx.diag_.error(alias.start_loc(), "Argument 'alias': Expected target or id or path expression");
      throw std::runtime_error("Sematic error");
   }

   ctx.current_project_.get_engine()->add_project_alias(&ctx.current_project_, project_alias, location.string(), props);
}

static
feature_attributes
resolve_attributes(const std::vector<std::string>& attributes)
{
   typedef std::vector<std::string>::const_iterator iter;
   feature_attributes result = {0};

   iter i = find(attributes.begin(), attributes.end(), "propagated");
   if (i != attributes.end())
      result.propagated = true;

   i = find(attributes.begin(), attributes.end(), "composite");
   if (i != attributes.end())
      result.composite = true;

   i = find(attributes.begin(), attributes.end(), "free");
   if (i != attributes.end())
      result.free = true;

   i = find(attributes.begin(), attributes.end(), "path");
   if (i != attributes.end())
      result.path = true;

   i = find(attributes.begin(), attributes.end(), "incidental");
   if (i != attributes.end())
      result.incidental = true;

   i = find(attributes.begin(), attributes.end(), "optional");
   if (i != attributes.end())
      result.optional = true;

   i = find(attributes.begin(), attributes.end(), "symmetric");
   if (i != attributes.end())
      result.symmetric = true;

   i = find(attributes.begin(), attributes.end(), "dependency");
   if (i != attributes.end())
      result.dependency = true;

   i = find(attributes.begin(), attributes.end(), "no-defaults");
   if (i != attributes.end())
      result.no_defaults = true;

   i = find(attributes.begin(), attributes.end(), "no-checks");
   if (i != attributes.end())
      result.no_checks = true;

   i = find(attributes.begin(), attributes.end(), "generated");
   if (i != attributes.end())
      result.generated = true;

   return result;
}

static
std::vector<std::string>
ast2list_of_ids(invocation_context& ctx,
                const ast::expression* e)
{
   std::vector<std::string> result;
   if (!e)
      return result;

   else if (const ast::id_expr* id = ast::as<ast::id_expr>(e)) {
      result.push_back(id->id().to_string());
   } else if (const ast::list_of* list = ast::as<ast::list_of>(e)) {
      for (auto le : list->values()) {
         if (const ast::id_expr* id = ast::as<ast::id_expr>(le))
            result.push_back(id->id().to_string());
         else {
            ctx.diag_.error(le->start_loc(), "Expected id or list of ids expression");
            throw std::runtime_error("Sematic error");
         }
      }
   } else {
      ctx.diag_.error(e->start_loc(), "Expected id or list of ids expression");
      throw std::runtime_error("Sematic error");
   }

   return result;
}

static
void feature_feature_rule(invocation_context& ctx,
                          const parscore::identifier& name,
                          const ast::expression* ast_values,
                          const ast::expression* ast_attributes)
{
   std::vector<std::string> values = ast2list_of_ids(ctx, ast_values);
   std::vector<std::string> attributes = ast2list_of_ids(ctx, ast_attributes);
   ctx.current_project_.get_engine()->feature_registry().add_feature_def(name.to_string(), values, resolve_attributes(attributes));
}

static
void feature_local_rule(invocation_context& ctx,
                        const parscore::identifier& name,
                        const ast::expression* ast_values,
                        const ast::expression* ast_attributes)
{
   std::vector<std::string> values = ast2list_of_ids(ctx, ast_values);
   std::vector<std::string> attributes = ast2list_of_ids(ctx, ast_attributes);
   ctx.current_project_.local_feature_registry().add_feature_def(name.to_string(), values, resolve_attributes(attributes));
}

static
void feature_compose_rule(invocation_context& ctx,
                          const feature& f,
                          const feature_or_feature_set_t& components)
{
   feature_registry& fr = ctx.current_project_.get_engine()->feature_registry();
   feature_set* cc;
   if (auto f = boost::get<const feature*>(&components)) {
      cc = fr.make_set();
      cc->join(fr.clone_feature(**f));
   } else
      cc = (**boost::get<const feature_set*>(&components)).clone();

   fr.get_def(f.definition().name()).compose(f.value(), cc);
}

void install_builtin_rules(rule_manager& rm)
{
   rm.add_rule("project", project_rule, {"id", "requirements", "usage-requirements"});
   rm.add_rule("use-project", use_project_rule, {"alias", "location", "requirements"});
   rm.add_rule("feature.feature", feature_feature_rule, {"name", "values", "attributes"});
   rm.add_rule("feature.local", feature_local_rule, {"name", "values", "attributes"});
   rm.add_rule("feature.compose", feature_compose_rule, {"feature", "components"});
   rm.add_target("exe", exe_rule, {"id", "sources", "requirements", "default-build", "usage-requirements"});
   rm.add_target("lib", lib_rule, {"id", "sources", "requirements", "default-build", "usage-requirements"});
   rm.add_target("alias", alias_rule, {"id", "sources", "requirements", "usage-requirements"});
}

}}