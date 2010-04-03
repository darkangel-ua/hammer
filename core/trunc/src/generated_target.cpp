#include "stdafx.h"
#include <hammer/core/generated_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/build_environment.h>

namespace hammer{

generated_target::generated_target(const main_target* mt, const pstring& n, 
                                   const target_type* t, const feature_set* f) : file_target(mt, n, t, f)
{
}

const location_t& generated_target::location() const
{
   return get_main_target()->intermediate_dir();
}

std::vector<boost::intrusive_ptr<build_node> > generated_target::generate()
{
   throw std::logic_error("generated target cannot be generated again.");
}

void generated_target::clean(const build_environment& environment) const
{
   environment.remove(location() / name().to_string());
}

}