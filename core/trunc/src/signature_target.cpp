#include "stdafx.h"
#include "signature_target.h"
#include <hammer/core/feature_set.h>
#include <hammer/core/main_target.h>
#include <fstream>

namespace hammer{

void signature_target::timestamp_info_impl() const
{
   timestamp_info_.is_unknown_ = false;

   std::string signature = dump_for_hash(get_main_target()->properties(), true);
   location_t l = location() / name().to_string();
   l.normalize();
   
   std::ifstream f(l.native_file_string().c_str());
   
   if (!f)
      timestamp_info_.timestamp_ = boost::date_time::neg_infin;
   else
   {
      std::string saved_signature;
      if (!getline(f, saved_signature))
         timestamp_info_.timestamp_ = boost::date_time::neg_infin;
      else
         if (signature != saved_signature)
            timestamp_info_.timestamp_ = boost::date_time::neg_infin;
         else
            generated_target::timestamp_info_impl();
   }
}

}
