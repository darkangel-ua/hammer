#include "stdafx.h"
#include <hammer/core/version_alias_meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>

namespace hammer{
   
static
source_decl make_default_source(engine& e,
                                const pstring& version)
{
   std::string location(version.to_string() + "/build");
   return source_decl(pstring(e.pstring_pool(), location), pstring(), NULL, NULL);
}

static
source_decl make_non_default_source(engine& e,
                                    const pstring& target_path)
{
   const location_t source_target_path = location_t("./") / target_path.to_string();
   return source_decl(pstring(e.pstring_pool(), source_target_path.string()), pstring(), NULL, NULL);
}

version_alias_meta_target::version_alias_meta_target(hammer::project* p,
                                                     const pstring& name,
                                                     const pstring& version,
                                                     const pstring* target_path)
   : alias_meta_target(p, name, sources_decl(), requirements_decl(), requirements_decl())
{
   requirements_decl reqs;
   reqs.add(*get_engine()->feature_registry().create_feature("version", version.to_string()));
   requirements(reqs);

   sources_decl src;
   source_decl s(target_path != NULL ? make_non_default_source(*get_engine(), *target_path)
                                     : make_default_source(*get_engine(), version));

   src.push_back(s);
   this->sources(src);
}

void version_alias_meta_target::instantiate_impl(const main_target* owner,
                                                 const feature_set& build_request,
                                                 std::vector<basic_target*>* result,
                                                 feature_set* usage_requirements) const
{
   feature_set& adjusted_build_request = *build_request.clone();
   adjusted_build_request.erase_all("version");

   alias_meta_target::instantiate_impl(owner, adjusted_build_request, result, usage_requirements);
}

}
