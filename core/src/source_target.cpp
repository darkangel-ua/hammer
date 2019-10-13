#include <hammer/core/source_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/source_build_target.h>

namespace hammer{

source_target::source_target(const main_target* mt,
                             const location_t& l,
                             const std::string& name,
                             const target_type* t,
                             const feature_set* f)
   : basic_target(mt, name, t, f),
     location_(l)
{
}

build_nodes_t
source_target::generate_impl() const
{
   boost::intrusive_ptr<build_node> result(new build_node(*get_main_target(), false, build_action_ptr{}));
   result->products_.push_back(new source_build_target(get_main_target(), name(), location_, &type(), &properties()));
   result->targeting_type_ = &this->type();
   return {1, result};
}

}
