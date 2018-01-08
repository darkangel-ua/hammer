#include <hammer/core/generated_build_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/build_environment.h>

namespace hammer{

generated_build_target::generated_build_target(const main_target* mt,
                                               const std::string& n,
                                               const target_type* t,
                                               const feature_set* f)
   : file_build_target(mt, n, t, f)
{
}

generated_2_build_target::generated_2_build_target(const main_target* mt,
                                                   const std::string& n,
                                                   const location_t& l,
                                                   const target_type* t,
                                                   const feature_set* f)
   : generated_build_target(mt, n, t, f),
     location_(l)
{
}

const location_t&
generated_build_target::location() const
{
   return get_main_target()->intermediate_dir();
}

void generated_build_target::clean(const build_environment& environment) const
{
   environment.remove(location() / name());
}

}
