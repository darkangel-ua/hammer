#include "stdafx.h"
#include "directory_target.h"
#include "main_target.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "types.h"
#include <stdexcept>
#include <boost/date_time/posix_time/conversion.hpp>

namespace hammer{

directory_target::directory_target(const main_target* mt, const location_t& dir)
   : basic_target(mt, pstring(), &mt->meta_target()->project()->engine()->get_type_registry().get(types::UNKNOWN), &mt->properties()),
     dir_to_create_(dir), action_(dir)
{

}

std::vector<boost::intrusive_ptr<build_node> > directory_target::generate()
{
   throw std::runtime_error("[directory_target::generate] This method must not be invoked");
}

void directory_target::timestamp_info_impl() const
{
   timestamp_info_.is_unknown_ = false;
   if (exists(dir_to_create_))
      timestamp_info_.timestamp_ = boost::posix_time::from_time_t(last_write_time(dir_to_create_));
   else
      timestamp_info_.timestamp_ = boost::posix_time::ptime(boost::date_time::neg_infin);
}

}
