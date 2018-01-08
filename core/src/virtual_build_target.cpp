#include <hammer/core/virtual_build_target.h>

namespace hammer {

virtual_build_target::virtual_build_target(const main_target* mt,
                                           const std::string& name,
                                           const target_type* t,
                                           const feature_set* f,
                                           const bool exists)
   : basic_build_target(mt, name, t, f)
{
   timestamp_info_.timestamp_ = exists ? boost::posix_time::ptime(boost::gregorian::date(1900, 1, 1))
                                       : boost::date_time::neg_infin;
}

static
location_t empty_location;

const location_t&
virtual_build_target::location() const
{
   return empty_location;
}

void virtual_build_target::timestamp_info_impl() const
{
   timestamp_info_.is_unknown_ = false;
}

}
