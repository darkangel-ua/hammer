#include "stdafx.h"
#include <hammer/core/default_output_location_strategy.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/feature_set.h>
#include <boost/crypto/md5.hpp>
#include <iostream>

using namespace std;

namespace hammer{

location_t default_output_location_strategy::compute_output_location(const main_target& mt) const
{
   ostringstream s;
   dump_for_hash(s, mt.properties());
   boost::crypto::md5 md5(s.str());
   return mt.meta_target()->project()->location() / ".hammer/bin" / mt.name().to_string() / md5.to_string();
}

}
