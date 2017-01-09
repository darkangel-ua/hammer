#include <hammer/core/target_version_alias_meta_target.h>

namespace hammer {

target_version_alias_meta_target::target_version_alias_meta_target(project* p,
                                                                   const pstring& name,
                                                                   const pstring& version,
                                                                   const pstring* target_path)
   : version_alias_meta_target(p, name, version, target_path)
{
   set_explicit(true);
   sources_decl sd = sources();
   sd.begin()->target_name(name);
   sources(sd);
}

}
