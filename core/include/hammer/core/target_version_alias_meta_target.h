#ifndef HAMMER_CORE_TARGET_VERSION_ALIAS_META_TARGET
#define HAMMER_CORE_TARGET_VERSION_ALIAS_META_TARGET

#include <hammer/core/version_alias_meta_target.h>

namespace hammer {

class target_version_alias_meta_target : public version_alias_meta_target
{
   public:
      target_version_alias_meta_target(hammer::project* p,
                                       const pstring& name,
                                       const pstring& version,
                                       const pstring* target_path);
};

}

#endif
