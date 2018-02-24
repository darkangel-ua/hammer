#include <hammer/core/generated_build_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/build_environment.h>

namespace hammer {

generated_build_target::generated_build_target(const main_target* mt,
                                               const std::string& target_name,
                                               const std::string& hash,
                                               const target_type* t,
                                               const feature_set* f,
                                               const location_t* l)
   : file_build_target(mt, target_name, t, f),
     hash_(hash),
     location_(l ? *l : mt->intermediate_dir() / hash_)
{
}

void generated_build_target::clean(const build_environment& environment) const
{
   environment.remove(location() / name());
}

}
