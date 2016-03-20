#include "stdafx.h"
#include <hammer/core/fake_target.h>

namespace hammer{

static location_t empty_location;

fake_target::fake_target(const main_target* mt, 
                         const build_node::sources_t& sources,
                         const pstring& name,
                         const target_type* t, 
                         const feature_set* f)
   : basic_target(mt, name, t, f), sources_(sources)
{
}

build_nodes_t fake_target::generate() const
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
   timestamp_info_.timestamp_ = boost::date_time::neg_infin;
   for(const build_node::source_t& s : sources_)
      timestamp_info_.timestamp_ = (std::max)(timestamp_info_.timestamp_, s.source_target_->timestamp_info().timestamp_);
}

}
