#include "builtin_rules.h"
#include <hammer/core/rule_manager.h>
#include <hammer/core/sources_decl.h>
#include <hammer/core/requirements_decl.h>
#include <hammer/core/typed_meta_target.h>
#include <hammer/core/types.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/alias_meta_target.h>

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
void exe_rule(invocation_context& ctx,
              const parscore::identifier& id,
              const sources_decl& sources,
              const requirements_decl* requirements,
              const feature_set* default_build,
              const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new typed_meta_target(&ctx.current_project_, id.to_string(), requirements ? *requirements : requirements_decl(),
                                                        usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl(),
                                                        ctx.current_project_.get_engine()->get_type_registry().get(types::EXE)));
   mt->sources(sources);
   ctx.current_project_.add_target(mt);
}

static
void alias_rule(invocation_context& ctx,
                const parscore::identifier& name,
                const sources_decl& sources,
                const requirements_decl* requirements,
                const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new alias_meta_target(&ctx.current_project_, name.to_string(),
                                                        sources,
                                                        requirements ? *requirements : requirements_decl(),
                                                        usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   ctx.current_project_.add_target(mt);
}


void install_builtin_rules(rule_manager& rm)
{
   rm.add_rule("project", project_rule, {"id", "requirements", "usage-requirements"});
   rm.add_rule("exe", exe_rule, {"id", "sources", "requirements", "default-build", "usage-requirements"});
   rm.add_rule("alias", alias_rule, {"id", "sources", "requirements", "usage-requirements"});
}

}}
