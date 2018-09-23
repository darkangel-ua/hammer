#pragma once
#include <hammer/core/searched_lib_meta_target.h>

namespace hammer {

class prebuilt_lib_meta_target : public searched_lib_meta_target {
   public:
      prebuilt_lib_meta_target(hammer::project* p,
                               const std::string& name,
                               const std::string& lib_file_path,
                               const requirements_decl& props,
                               const requirements_decl& usage_req);
};

}
