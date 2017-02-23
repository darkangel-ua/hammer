#include "stdafx.h"
#include <hammer/core/version_alias_meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>

namespace hammer{
   
static
source_decl make_default_source(const std::string& version)
{
   std::string location("./" + version + "/build");
   return source_decl(location, std::string(), NULL, NULL);
}

static
source_decl make_non_default_source(const std::string& target_path)
{
   const location_t source_target_path = location_t("./") / target_path;
   return source_decl(source_target_path.string(), std::string(), NULL, NULL);
}

version_alias_meta_target::version_alias_meta_target(hammer::project* p,
                                                     const std::string& name,
                                                     const std::string& version,
                                                     const std::string* target_path)
   : alias_meta_target(p, name, sources_decl(), requirements_decl(), requirements_decl())
{
   requirements_decl reqs;
   reqs.add(*get_engine()->feature_registry().create_feature("version", version));
   requirements(reqs);

   sources_decl src;
   source_decl s(target_path != NULL ? make_non_default_source(*target_path)
                                     : make_default_source(version));

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
