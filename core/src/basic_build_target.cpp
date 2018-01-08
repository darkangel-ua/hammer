#include <hammer/core/basic_build_target.h>
#include <hammer/core/main_target.h>

namespace hammer{

const basic_meta_target*
basic_build_target::get_meta_target() const
{
   return get_main_target()->get_meta_target();
}

const project*
basic_build_target::get_project() const
{
   return get_main_target()->get_project();
}

engine*
basic_build_target::get_engine() const
{
   return get_main_target()->get_engine();
}

const location_t&
basic_build_target::location() const
{
   return get_main_target()->location();
}

const timestamp_info_t&
basic_build_target::timestamp_info() const
{
   if (!timestamp_info_.is_unknown_)
      return timestamp_info_;

   timestamp_info_impl();

   return timestamp_info_;
}

}
