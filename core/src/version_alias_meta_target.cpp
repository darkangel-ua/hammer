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
                                    const source_decl& source)
{
   const location_t source_target_path = location_t("./") / source.target_path().to_string();
   return source_decl(pstring(e.pstring_pool(), source_target_path.string()), pstring(), NULL, NULL);
}

version_aliase_meta_target::version_aliase_meta_target(hammer::project* p,
                                                       const pstring& name,
                                                       const pstring& version,
                                                       const sources_decl* sources)
   : alias_meta_target(p, name, sources_decl(), requirements_decl(), requirements_decl())
{
   requirements_decl reqs;
   reqs.add(*get_engine()->feature_registry().create_feature("version", version.to_string()));
   requirements(reqs);

   feature_set* props = get_engine()->feature_registry().make_set();
   props->join("version", version.begin());

   sources_decl src;
   source_decl s(sources != NULL ? make_non_default_source(*get_engine(), *sources->begin())
                                 : make_default_source(*get_engine(), version));

   s.properties(s.properties() == NULL ? props : &s.properties()->join(*props));
   src.push_back(s);
   this->sources(src);
}

}
