#include "stdafx.h"
#include <hammer/core/header_lib_target.h>

namespace hammer{

header_lib_target::header_lib_target(const main_target* mt, const pstring& name,
                                     const target_type* t, const feature_set* f)
   : basic_target(mt, name, t, f)
{

}

std::vector<boost::intrusive_ptr<build_node> > header_lib_target::generate()
{
   throw std::runtime_error("[header_lib_target] Generate should not be invoked");
}

void header_lib_target::timestamp_info_impl() const
{
   timestamp_info_.is_unknown_ = false;
   // because it is system library we know nothing about it existence
   // but we cannot return neg_infin, so we return very old times :)
   timestamp_info_.timestamp_ = boost::posix_time::ptime(boost::gregorian::date(1900, 1, 1));
}

}
