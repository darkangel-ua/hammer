#include "stdafx.h"
#include <hammer/core/basic_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/feature_set.h>
#include <boost/crypto/md5.hpp>

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

void basic_target::properties(const feature_set* p) 
{ 
   hash_.reset();
   features_ = p; 
}

const std::string& basic_target::hash_string() const
{
   if (!hash_)
      hash_ = hash_string(properties(), *mtarget());
   
   return *hash_;
}

std::string basic_target::hash_string(const feature_set& fs, const main_target& mt)
{
   std::ostringstream s;
   dump_for_hash(s, fs);
   static_cast<const basic_target&>(mt).additional_hash_string_data(s);
   return boost::crypto::md5(s.str()).to_string();
}

}