#pragma once
#include <hammer/core/alias_meta_target.h>

namespace hammer {

class testing_suite_meta_target : public alias_meta_target
{
   public:
      testing_suite_meta_target(hammer::project* p,
                                const std::string& name,
                                const sources_decl& sources,
                                const sources_decl& common_sources);
};

}
