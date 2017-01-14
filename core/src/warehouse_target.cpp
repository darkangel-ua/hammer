#include <hammer/core/warehouse_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/engine.h>

namespace hammer {

warehouse_target::warehouse_target(const basic_meta_target& mt,
                                   const pstring& name,
                                   const feature_set& build_request)
   : main_target(&mt, name, NULL, build_request.clone(), mt.get_engine()->pstring_pool())
{

}

warehouse_target::~warehouse_target()
{

}

build_nodes_t warehouse_target::generate() const
{
   throw warehouse_unresolved_target_exception();
}

warehouse_unresolved_target_exception::warehouse_unresolved_target_exception()
   : std::runtime_error("Build tree contain unresolved warehouse lib")
{
}

}
