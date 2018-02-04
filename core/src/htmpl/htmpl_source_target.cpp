#include "htmpl_source_target.h"
#include "htmpl_build_target.h"

namespace hammer {

build_nodes_t
htmpl_source_target::generate() const
{
   build_node_ptr result(new build_node(*get_main_target(), false, {}));
   result->products_.push_back(new htmpl_build_target(get_main_target(), name(), location(), &type(), &properties()));
   result->targeting_type_ = &this->type();
   return build_nodes_t(1, result);
}

}
