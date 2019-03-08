#include <hammer/core/testing_suite_meta_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/project.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature.h>
#include <hammer/core/feature_set.h>

namespace hammer {

static
sources_decl
adjust_sources(const project& p,
               const sources_decl& sources,
               const sources_decl& common_sources) {
   feature_registry& fr = p.get_engine().feature_registry();
   sources_decl modified_sources{sources};

   if (!common_sources.empty()) {
      feature_set& build_request = *fr.make_set();
      for (auto& s : common_sources) {
         feature& f = *fr.create_feature("testing.additional-source", "");
         f.set_dependency_data(s, &p);
         build_request.join(&f);
      }

      modified_sources.add_to_source_properties(build_request);
   }

   return modified_sources;
}

testing_suite_meta_target::testing_suite_meta_target(hammer::project* p,
                                                     const std::string& name,
                                                     const sources_decl& sources,
                                                     const sources_decl& common_sources)
   : alias_meta_target(p, name, adjust_sources(*p, sources, common_sources), {}, {})
{

}

}
