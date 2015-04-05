#include <hammer/core/warehouse_target.h>
#include <hammer/core/feature_set.h>

namespace hammer {

warehouse_target::warehouse_target(const main_target& mt,
                                   const pstring& name,
                                   const feature_set& build_request)
   : basic_target(&mt, name, NULL, build_request.clone())
{

}

warehouse_target::~warehouse_target()
{

}

build_nodes_t warehouse_target::generate() const
{
   throw warehouse_unresolved_target_exception();
}

void warehouse_target::timestamp_info_impl() const
{
   throw std::runtime_error("warehouse_target: can't do timestamp_info_impl");
}

warehouse_unresolved_target_exception::warehouse_unresolved_target_exception()
   : std::runtime_error("Build tree contain unresolved warehouse lib")
{
}

warehouse_unresolved_target_exception::~warehouse_unresolved_target_exception() throw()
{
}

}
