#include <hammer/core/warehouse_meta_target.h>
#include <hammer/core/warehouse_target.h>

using namespace std;

namespace hammer {

warehouse_meta_target::warehouse_meta_target(project& p,
                                             const pstring& name)
   : basic_meta_target(&p, name, requirements_decl(), requirements_decl())
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
   auto_ptr<basic_target> t(new warehouse_target(*owner, name(), build_request));
   result->push_back(t.get());
   t.release();
}

}
