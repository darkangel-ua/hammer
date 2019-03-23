#include <hammer/core/engine.h>
#include <hammer/core/project.h>
#include <hammer/core/types.h>
#include <hammer/core/testing_suite_meta_target.h>

namespace hammer {

testing_suite_meta_target::testing_suite_meta_target(hammer::project& p,
                                                     const std::string& name,
                                                     const sources_decl& sources,
                                                     const sources_decl& common_sources)
   : typed_meta_target(&p, name, {}, {}, p.get_engine().get_type_registry().get(types::TESTING_SUITE)),
     common_sources_(common_sources)
{
   this->sources(sources);
}

void testing_suite_meta_target::common_sources(const sources_decl& s) {
   common_sources_ = s;
}

}
