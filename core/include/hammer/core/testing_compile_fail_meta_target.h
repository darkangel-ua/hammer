#pragma once
#include <hammer/core/testing_compile_base_meta_target.h>

namespace hammer {

class testing_compile_fail_meta_target : public testing_compile_base_meta_target {
   public:
      testing_compile_fail_meta_target(hammer::project* p,
                                       const std::string& name,
                                       const requirements_decl& req);
};

}
