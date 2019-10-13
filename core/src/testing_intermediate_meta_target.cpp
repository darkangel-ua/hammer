#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/testing_intermediate_meta_target.h>

namespace hammer {

testing_intermediate_meta_target::testing_intermediate_meta_target(hammer::project* p, 
                                                                   const std::string& name,
                                                                   const requirements_decl& req)
   : testing_link_base_meta_target(p, name, req, p->get_engine().get_type_registry().get(types::EXE))
{
}

main_target*
testing_intermediate_meta_target::construct_main_target(const main_target* owner,
                                                        const feature_set* properties) const
{
   // we don't need to construct testing_main_target as testing_base_meta_target will do
   // we just need regular main_target
   return typed_meta_target::construct_main_target(owner, properties);
}

testing_intermediate_meta_target::~testing_intermediate_meta_target() = default;

}
