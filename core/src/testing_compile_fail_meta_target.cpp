#include <hammer/core/testing_compile_fail_meta_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/testing_main_target.h>

namespace hammer {

testing_compile_fail_meta_target::testing_compile_fail_meta_target(project* p,
                                                                   const pstring& name,
                                                                   const requirements_decl& req,
                                                                   const requirements_decl& usage_req)
   : typed_meta_target(p, name, req, usage_req, p->get_engine()->get_type_registry().get(types::TESTING_COMPILE_FAIL))
{
}

}
