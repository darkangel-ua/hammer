#include <hammer/core/testing_base_meta_target.h>
#include <hammer/core/testing_main_target.h>

namespace hammer {

testing_base_meta_target::testing_base_meta_target(hammer::project* p,
                                                   const std::string& name,
                                                   const requirements_decl& req,
                                                   const target_type& t)
   : typed_meta_target(p, name, req, {}, t)
{
}

main_target*
testing_base_meta_target::construct_main_target(const main_target* owner,
                                                const feature_set* properties) const
{
   return new testing_main_target{this, name(), &type(), properties};
}

}
