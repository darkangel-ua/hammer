#include "stdafx.h"
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <hammer/core/file_target.h>
#include <hammer/core/main_target.h>

namespace hammer
{

std::vector<boost::intrusive_ptr<build_node> > file_target::generate()
{
   return std::vector<boost::intrusive_ptr<build_node> >();
}

void file_target::timestamp_info_impl() const
{
   boost::filesystem::path p(location() / name().to_string());
   timestamp_info_.is_unknown_ = false;
   if (exists(p))
      timestamp_info_.timestamp_ = boost::posix_time::from_time_t(last_write_time(p));
   else
      timestamp_info_.timestamp_ = boost::posix_time::ptime(boost::date_time::neg_infin);
}

}
