#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <hammer/core/file_build_target.h>

namespace hammer {

void file_build_target::timestamp_info_impl() const
{
   boost::filesystem::path p(location() / name());
   timestamp_info_.is_unknown_ = false;
   if (exists(p))
      timestamp_info_.timestamp_ = boost::posix_time::from_time_t(last_write_time(p));
   else
      timestamp_info_.timestamp_ = boost::posix_time::neg_infin;
}

}
