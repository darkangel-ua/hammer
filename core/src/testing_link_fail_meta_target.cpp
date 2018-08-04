#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/testing_link_fail_meta_target.h>

namespace hammer {

testing_link_fail_meta_target::testing_link_fail_meta_target(project* p,
                                                             const std::string& name,
                                                             const requirements_decl& req)
   : testing_link_base_meta_target(p, name, req, p->get_engine().get_type_registry().get(types::TESTING_LINK_FAIL))
{
}

}
