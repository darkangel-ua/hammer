#include "stdafx.h"
#include "basic_target.h"
#include "main_target.h"

namespace hammer{

const timestamp_info_t& 
basic_target::timestamp_info(timestamp_info_t::getter_policy_t how_to_get) const
{
   if (how_to_get == timestamp_info_t::just_get)
      return timestamp_info_;
   
   timestamp_info_impl();
   
   return timestamp_info_;
}

const location_t& basic_target::location() const
{
   return mtarget()->location();
}

}