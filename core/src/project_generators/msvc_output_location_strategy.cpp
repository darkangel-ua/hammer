#include "stdafx.h"
#include <hammer/core/project_generators/msvc_output_location_strategy.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <boost/crypto/md5.hpp>
#include <iostream>

using namespace std;

namespace hammer{

location_t msvc_output_location_strategy::compute_output_location(const main_target& mt) const
{
   ostringstream s;
   dump_for_hash(s, mt.properties());
   boost::crypto::md5 md5(s.str());
   std::string variant_name("unknown");
   const feature_set::const_iterator i_variant = mt.properties().find("variant");
   if (i_variant != mt.properties().end())
      variant_name = (**i_variant).name();

   return mt.get_project()->location() / ".hammer/vc80" / mt.get_meta_target()->name() / (variant_name + '-' + md5.to_string());
}

}
