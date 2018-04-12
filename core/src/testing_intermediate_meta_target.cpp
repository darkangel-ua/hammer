#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/main_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/testing_intermediate_meta_target.h>

namespace hammer {

testing_intermediate_meta_target::testing_intermediate_meta_target(hammer::project* p, 
                                                                   const std::string& name,
                                                                   const requirements_decl& req,
                                                                   const args& args)
   : typed_meta_target(p, name, req, {}, p->get_engine()->get_type_registry().get(types::EXE)),
     args_(args)
{
}

sources_decl
testing_intermediate_meta_target::compute_additional_sources(const main_target& owner) const
{
   sources_decl result;
   for (auto i = owner.properties().find("testing.additional-source"), last = owner.properties().end(); i != last;) {
      result.push_back((**i).get_dependency_data().source_);
      i = owner.properties().find(i + 1, "testing.additional-source");
   }

   return result;
}

testing_intermediate_meta_target::~testing_intermediate_meta_target()
{
}

}
