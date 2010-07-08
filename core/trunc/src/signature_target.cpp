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
      std::copy(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>(), back_inserter(saved_signature));
      if (!f)
         timestamp_info_.timestamp_ = boost::date_time::neg_infin;
      else
         if (signature != saved_signature)
            timestamp_info_.timestamp_ = boost::date_time::neg_infin;
         else
            // we can't do generated_target::timestamp_info_impl();
            // because target.sig will we older than sources that depends on it
            // and this will cause constant rebuild
            timestamp_info_.timestamp_ = boost::posix_time::ptime(boost::gregorian::date(1900, 1, 1));
   }
}

}
