#include <hammer/core/source_decl.h>
#include <hammer/core/build_request.h>

namespace hammer {

source_decl::source_decl() : project_(*((const project*)0)), type_(nullptr) {}

source_decl::source_decl(const project& p,
                         const std::string& target_path,
                         const std::string& target_name,
                         const target_type* t)
   :
     project_(p),
     target_path_(target_path),
     target_name_(target_name),
     type_(t)
{
}

source_decl::source_decl(const source_decl& v) = default;
source_decl::source_decl(source_decl&& v) = default;
source_decl& source_decl::operator = (const source_decl& rhs) = default;

void source_decl::build_request(const hammer::build_request& v) {
   build_request_ = v;
}

void source_decl::build_request(hammer::build_request&& v) {
   build_request_ = std::move(v);
}

void source_decl::reset_build_request() {
   build_request_.reset();
}

void source_decl::build_request_join(const feature_set& props) {
   if (build_request())
      build_request()->join(props);
   else
      build_request(hammer::build_request{props});
}

source_decl::~source_decl() = default;

bool looks_like_local_target_ref(const source_decl& sd) {
   if (!sd.is_meta_target() || sd.is_project_local_reference())
      return false;

   return (sd.target_path() == "./" && !sd.target_name().empty()) ||
          (sd.target_name().empty() && sd.target_path().find('/') == std::string::npos);
}

bool source_decl::is_project_local_reference() const
{
   return !target_path().empty() && target_path().front() == '^';
}

}
