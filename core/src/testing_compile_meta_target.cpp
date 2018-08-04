#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/testing_compile_meta_target.h>

namespace hammer {

testing_compile_meta_target::testing_compile_meta_target(project* p,
                                                         const std::string& name,
                                                         const requirements_decl& req)
   : testing_compile_base_meta_target(p, name, req, p->get_engine().get_type_registry().get(types::TESTING_COMPILE_SUCCESSFUL))
{
}

}
