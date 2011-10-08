#include "stdafx.h"
#include <stdexcept>
#include <boost/filesystem/convenience.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <hammer/core/directory_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/types.h>

namespace hammer{

directory_target::directory_target(const main_target* mt, const location_t& dir)
   : basic_target(mt, pstring(mt->get_engine()->pstring_pool(), dir.string()), &mt->get_engine()->get_type_registry().get(types::UNKNOWN), &mt->properties()),
     dir_to_create_(dir), action_(dir)
{

}

std::vector<boost::intrusive_ptr<build_node> > directory_target::generate() const
{
   throw std::runtime_error("[directory_target::generate] This method must not be invoked");
}

void directory_target::timestamp_info_impl() const
{
   timestamp_info_.is_unknown_ = false;
   // if directory exists we don't care about its timestamp. 
   // We just say that it was created very long time ago.
   if (exists(dir_to_create_))
      timestamp_info_.timestamp_ = boost::posix_time::ptime(boost::gregorian::date(1900, 1, 1));
   else
      timestamp_info_.timestamp_ = boost::posix_time::ptime(boost::date_time::neg_infin);
}

}
