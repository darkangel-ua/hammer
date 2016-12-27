#include "htmpl_meta_target.h"
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/main_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/target_type.h>
#include <hammer/core/types.h>
#include <hammer/core/fs_helpers.h>
#include <stdexcept>

using namespace std;
namespace fs = boost::filesystem;

namespace hammer {

static
const target_type&
resolve_target_type(const fs::path& filename,
                    engine& e)
{
   const fs::path target_source = filename.stem();
   const pstring p_target_source = pstring(e.pstring_pool(), target_source.string());
   const target_type* target_source_type = e.get_type_registry().resolve_from_target_name(p_target_source);

   if (!target_source_type)
      throw std::runtime_error("[htmp_meta_target] Can't resolve source type from '" + filename.string() + "'");

   return *target_source_type;
}

static
requirements_decl
make_usage_requirements(feature_registry& fr,
                        const pstring& target_name,
                        const basic_meta_target* this_)
{
   requirements_decl result;

   // making dependency on self :)
   feature* dependency = fr.create_feature("dependency", "");
   dependency->set_dependency_data(source_decl(target_name, pstring(), nullptr, nullptr), this_);
   result.add(*dependency);

   return result;
}

htmpl_meta_target::htmpl_meta_target(project* p,
                                     const pstring& name,
                                     const source_decl& src)
   : typed_meta_target(p,
                       name,
                       requirements_decl(),
                       make_usage_requirements(p->get_engine()->feature_registry(), name, this),
                       resolve_target_type(src.target_path().to_string(), *p->get_engine()))
{
   set_explicit(true);

   sources_decl sources;
   sources.push_back(src);
   add_sources(sources);
}

main_target*
htmpl_meta_target::construct_main_target(const main_target* owner,
                                         const feature_set* properties) const
{
   if (!owner)
      throw std::runtime_error("htmpl targets MUST not be used in alias-kind targets");

   const fs::path source = sources().begin()->target_path().to_string();
   const string main_target_name = source.filename().stem().stem().string();
   const pstring pmain_target_name(get_engine()->pstring_pool(), main_target_name);
   // we use owner properties because htmpl targets created without any properties at all
   main_target* mt = new main_target(this,
                                     pmain_target_name,
                                     &type(),
                                     &owner->properties(),
                                     get_engine()->targets_pool());
   return mt;
}

void htmpl_meta_target::compute_usage_requirements(feature_set& result,
                                                   const main_target& constructed_target,
                                                   const feature_set& build_request,
                                                   const feature_set& computed_usage_requirements,
                                                   const main_target* owner) const
{
   if (type().equal_or_derived_from(types::H)) {
      // generated include dir should apear first, so generated includes will be found earlier
      const fs::path relative_intermediate_dir_path = relative_path(constructed_target.intermediate_dir(), location());
      feature* include_intermediate_dir = get_engine()->feature_registry().create_feature("include", relative_intermediate_dir_path.string());
      include_intermediate_dir->get_path_data().target_ = this;
      result.join(include_intermediate_dir);
   }

   typed_meta_target::compute_usage_requirements(result, constructed_target, build_request, computed_usage_requirements, owner);
}

}
