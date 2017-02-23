#include <hammer/core/warehouse_meta_target.h>
#include <hammer/core/warehouse_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>

using namespace std;

namespace hammer {

static
requirements_decl
make_requirements(feature_registry& fr,
                  const string& version)
{
   requirements_decl result;
   result.add(*fr.create_feature("version", version));

   return result;
}

warehouse_meta_target::warehouse_meta_target(project& p,
                                             const std::string& name,
                                             const string& version)
   : meta_target(&p, name, make_requirements(p.get_engine()->feature_registry(), version), requirements_decl())
{
}

warehouse_meta_target::~warehouse_meta_target()
{

}

void warehouse_meta_target::instantiate_impl(const main_target* owner,
                                             const feature_set& build_request,
                                             std::vector<basic_target*>* result,
                                             feature_set* usage_requirements) const
{
   unique_ptr<basic_target> t(new warehouse_target(*this, name(), build_request));
   result->push_back(t.get());
   t.release();
}

main_target*
warehouse_meta_target::construct_main_target(const main_target* owner,
                                             const feature_set* properties) const
{
   assert(false);
   return nullptr;
}

}
