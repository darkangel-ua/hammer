#include <stdexcept>
#include <regex>
#include <boost/filesystem/fstream.hpp>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/main_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/target_type.h>
#include <hammer/core/types.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/source_target.h>
#include <hammer/core/source_build_target.h>
#include "htmpl_meta_target.h"
#include "htmpl_source_target.h"

using namespace std;
namespace fs = boost::filesystem;

namespace hammer {

static
const target_type&
resolve_target_type(const fs::path& filename,
                    engine& e)
{
   const fs::path target_source = filename.stem();
   const target_type* target_source_type = e.get_type_registry().resolve_from_target_name(target_source.string());

   if (!target_source_type)
      throw std::runtime_error("[htmp_meta_target] Can't resolve source type from '" + filename.string() + "'");

   return *target_source_type;
}

static
requirements_decl
make_usage_requirements(feature_registry& fr,
                        const std::string& target_name,
                        const basic_meta_target* this_)
{
   requirements_decl result;

   // making dependency on self :) because this will build this target before any targets that belongs to owner
   feature* dependency = fr.create_feature("dependency", "");
   dependency->set_dependency_data(source_decl{this_->get_project(), "./", target_name, nullptr, nullptr}, &this_->get_project());
   result.add(*dependency);

   return result;
}

htmpl_meta_target::htmpl_meta_target(project* p,
                                     const std::string& name,
                                     const source_decl& src)
   : typed_meta_target(p,
                       name,
                       requirements_decl(),
                       make_usage_requirements(p->get_engine().feature_registry(), name, this),
                       resolve_target_type(src.target_path(), p->get_engine()))
{
   set_explicit(true);
   set_local(true);

   sources_decl sources;
   sources.push_back(src);
   add_sources(sources);
}

main_target*
htmpl_meta_target::construct_main_target(const main_target* owner,
                                         const feature_set* properties) const
{
   const fs::path source = sources().begin()->target_path();
   const string main_target_name = source.filename().stem().string();
   // we use owner properties because htmpl targets created without any properties at all
   main_target* mt = new main_target(this,
                                     main_target_name,
                                     &type(),
                                     owner ? &owner->properties() : properties);
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
      feature* generated_include = get_engine().feature_registry().create_feature("__generated-include", "");
      generated_include->get_generated_data().target_ = &constructed_target;
      result.join(generated_include);
   }

   typed_meta_target::compute_usage_requirements(result, constructed_target, build_request, computed_usage_requirements, owner);
}

basic_target*
htmpl_meta_target::create_simple_target(const main_target& owner,
                                        const location_t& source_location,
                                        const target_type& tp,
                                        const feature_set* properties) const
{
   return new htmpl_source_target(&owner, source_location.branch_path(), source_location.filename().string(), &tp, properties);
}

}
