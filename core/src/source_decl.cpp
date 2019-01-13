#include <hammer/core/source_decl.h>

namespace hammer {

bool looks_like_local_target_ref(const source_decl& sd)
{
   return (sd.target_path() == "./" && !sd.target_name().empty()) ||
         (sd.target_name().empty() && sd.target_path().find('/') == std::string::npos);
}

bool source_decl::is_project_local_reference() const
{
   return !target_path().empty() && target_path().front() == '^';
}

}
