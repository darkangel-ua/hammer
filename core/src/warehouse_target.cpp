#include <hammer/core/warehouse_target.h>
#include <hammer/core/warehouse_manager.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/engine.h>

namespace hammer {

warehouse_target::warehouse_target(const basic_meta_target& mt,
                                   const std::string& name,
                                   const feature_set& build_request)
   : main_target(&mt, name, NULL, build_request.clone())
{

}

warehouse_target::~warehouse_target()
{

}

build_nodes_t
warehouse_target::generate_impl() const
{
   auto w = get_engine().warehouse_manager().find(get_project());
   throw warehouse_unresolved_target_exception(w->second.get());
}

warehouse_unresolved_target_exception::warehouse_unresolved_target_exception(warehouse* wh)
   : std::runtime_error("Build tree contain unresolved warehouse lib"),
     warehouse_(wh)
{
   assert(wh);
}

}
