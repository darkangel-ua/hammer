#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/header_lib_meta_target.h>

namespace hammer {

header_lib_meta_target::header_lib_meta_target(hammer::project* p,
                                               const std::string& name,
                                               const requirements_decl& requirements,
                                               const requirements_decl& usage_requirements)
   : typed_meta_target(p, name, requirements, usage_requirements, p->get_engine().get_type_registry().get(types::HEADER_LIB))
{

}

void header_lib_meta_target::compute_usage_requirements(feature_set& result,
                                                        const main_target& constructed_target,
                                                        const feature_set& build_request,
                                                        const feature_set& computed_usage_requirements,
                                                        const main_target* owner) const
{
   // we just tranfer all computed usage requirements for this target further up
   result.join(computed_usage_requirements);
}

}
