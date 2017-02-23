#include <hammer/core/virtual_target.h>

namespace hammer {

virtual_target::virtual_target(const main_target* mt,
                               const std::string& name,
                               const target_type* t,
                               const feature_set* f)
   : basic_target(mt, name, t, f)
{

}

build_nodes_t
virtual_target::generate() const
{
   throw std::runtime_error("virtual_target::generate - should be invoked");
}

void virtual_target::timestamp_info_impl() const
{
   timestamp_info_.is_unknown_ = false;
   timestamp_info_.timestamp_ = boost::date_time::neg_infin;
}

}
