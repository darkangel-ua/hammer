#include "stdafx.h"
#include <stdexcept>
#include <boost/filesystem/convenience.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <hammer/core/directory_build_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/build_environment.h>

namespace hammer{

directory_build_target::directory_build_target(const main_target* mt,
                                               const location_t& dir)
   : generated_build_target(mt, {}, dir.string(), &mt->get_engine()->get_type_registry().get(types::UNKNOWN), &mt->properties()),
     dir_to_create_(dir),
     action_(std::make_shared<mkdir_action>(dir))
{
}

void directory_build_target::timestamp_info_impl() const
{
   timestamp_info_.is_unknown_ = false;
   // if directory exists we don't care about its timestamp. 
   // We just say that it was created very long time ago.
   if (exists(dir_to_create_))
      timestamp_info_.timestamp_ = boost::posix_time::ptime(boost::gregorian::date(1900, 1, 1));
   else
      timestamp_info_.timestamp_ = boost::posix_time::ptime(boost::date_time::neg_infin);
}

void directory_build_target::clean(const build_environment& environment) const
{
   environment.remove(location());
}

static
std::string empty_hash;

const std::string&
directory_build_target::hash() const
{
   return empty_hash;
}

}
