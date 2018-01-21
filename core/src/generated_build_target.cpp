#include <hammer/core/generated_build_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/feature_set.h>

namespace hammer{

generated_build_target::generated_build_target(const main_target* mt,
                                               const std::string& n,
                                               const target_type* t,
                                               const feature_set* f,
                                               const location_t* l)
   : file_build_target(mt, n, t, f),
     location_(l ? *l : mt->intermediate_dir() / md5(*f, true))
{
}

void generated_build_target::clean(const build_environment& environment) const
{
   environment.remove(location() / name());
}

}
