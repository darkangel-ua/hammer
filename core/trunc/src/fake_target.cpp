#include "stdafx.h"
#include <hammer/core/fake_target.h>

namespace hammer{

static location_t empty_location;

fake_target::fake_target(const main_target* mt, const pstring& name,
                         const target_type* t, const feature_set* f)
   : basic_target(mt, name, t, f)
{

}

build_nodes_t fake_target::generate()
{
   throw std::runtime_error("[fake_target]: Generate should not be invoked");
}

const location_t& fake_target::location() const
{
   return empty_location;
}

void fake_target::timestamp_info_impl() const
{
   timestamp_info_.is_unknown_ = false;
   // because it is fake target it is always exists
   // but we cannot return neg_infin, so we return very old times :)
   timestamp_info_.timestamp_ = boost::posix_time::ptime(boost::gregorian::date(1900, 1, 1));
}

}
