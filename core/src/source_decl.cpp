#include <hammer/core/source_decl.h>

namespace hammer {

bool looks_like_local_target_ref(const source_decl& sd)
{
   return (sd.target_path() == "./" && !sd.target_name().empty()) ||
          (sd.target_name().empty() && sd.target_path().find('/') == std::string::npos);
}

}
