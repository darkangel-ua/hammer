#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/testing_main_target.h>
#include <hammer/core/testing_meta_target.h>

namespace hammer {

testing_meta_target::testing_meta_target(hammer::project* p,
                                         const std::string& name,
                                         const requirements_decl& req)
   : typed_meta_target(p, name, req, {}, p->get_engine()->get_type_registry().get(types::TESTING_RUN_PASSED))
{
}

main_target* 
testing_meta_target::construct_main_target(const main_target* owner, 
                                           const feature_set* properties) const
{
   main_target* mt = new testing_main_target(this, 
                                             name(), 
                                             &type(), 
                                             properties);
   return mt;
}

}
