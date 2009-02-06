#include "stdafx.h"
#include "searched_lib_target.h"

namespace hammer{

void searched_lib_target::timestamp_info_impl() const
{
   timestamp_info_.is_unknown_ = false;
   // because it is system library we know nothing about it existence
   // but we cannot return neg_infin, so we return very old times :)
   timestamp_info_.timestamp_ = boost::posix_time::ptime(boost::gregorian::date(1900, 1, 1));
}

}
