#include <hammer/core/fake_build_target.h>

namespace hammer {

static
location_t empty_location;

fake_build_target::fake_build_target(const main_target* mt,
                                     const build_node::sources_t& sources,
                                     const std::string& name,
                                     const target_type* t,
                                     const feature_set* f)
   : basic_build_target(mt, name, t, f),
     sources_(sources)
{
}

const location_t&
fake_build_target::location() const
{
   return empty_location;
}

void fake_build_target::timestamp_info_impl() const
{
   timestamp_info_.is_unknown_ = false;
   timestamp_info_.timestamp_ = boost::date_time::neg_infin;
   for(const build_node::source_t& s : sources_)
      timestamp_info_.timestamp_ = (std::max)(timestamp_info_.timestamp_, s.source_target_->timestamp_info().timestamp_);
}

}
