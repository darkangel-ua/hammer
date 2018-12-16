#include <hammer/core/basic_build_target.h>
#include <hammer/core/main_target.h>

namespace hammer{

basic_build_target::basic_build_target(const main_target* mt,
                                       const std::string& name,
                                       const target_type* t,
                                       const feature_set* f)
   : main_target_(mt),
     type_(t),
     name_(name),
     features_(f)
{
}

basic_build_target::~basic_build_target()
{
}

const basic_meta_target*
basic_build_target::get_meta_target() const
{
   return get_main_target()->get_meta_target();
}

const project&
basic_build_target::get_project() const
{
   return get_main_target()->get_project();
}

engine&
basic_build_target::get_engine() const
{
   return get_main_target()->get_engine();
}

const timestamp_info_t&
basic_build_target::timestamp_info() const
{
   if (!timestamp_info_.is_unknown_)
      return timestamp_info_;

   timestamp_info_impl();

   return timestamp_info_;
}

static
std::vector<const feature*> empty_valuable_features;

const std::vector<const feature*>&
basic_build_target::valuable_features() const
{
   return empty_valuable_features;
}

}
