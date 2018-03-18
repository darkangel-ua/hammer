#include <hammer/core/source_decl.h>

namespace hammer {

bool local_target(const source_decl& sd)
{
   return sd.target_path().empty() ||
          (sd.target_path() == "./" && !sd.target_name().empty()) ||
          (sd.target_name().empty() && sd.target_path().find('/') == std::string::npos);
}

}
