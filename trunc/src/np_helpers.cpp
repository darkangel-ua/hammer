#include "stdafx.h"
#include "np_helpers.h"
#include "pstring.h"
#include "type.h"
#include "feature_set.h"
#include <boost/crypto/md5.hpp>

using namespace std;

namespace hammer{

static string compute_hash(const feature_set& features)
{
   return boost::crypto::md5(dump_for_hash(features)).to_string();
}

pstring make_name(pool& p, 
                  const pstring& source_name, 
                  const type& source_type, 
                  const type& target_type,
                  const feature_set* target_properties)
{
   const std::string& source_suffix = source_type.suffix_for(source_name.to_string()); // FIXME: Here is conversion from pstring to string
   // find leaf in source name to leave only filename
   string::size_type slash_pos = source_name.find_last_of('/');
   if (slash_pos == string::npos)
      slash_pos = 0;

   std::string hash_suffix;
   if (target_properties != NULL)
      hash_suffix = '-' + compute_hash(*target_properties);
 
   return pstring(p, std::string(source_name.begin() + slash_pos, source_name.begin() + (source_name.size() - source_suffix.size())) + hash_suffix + *target_type.suffixes().begin());
}

pstring make_name(pool& p, 
                  const pstring& source_name, 
                  const type& target_type,
                  const feature_set* target_properties)
{
   if (target_type.suffixes().empty())
      return source_name;
   else
   {
      std::string hash_suffix;
      if (target_properties != NULL)
         hash_suffix = '-' + compute_hash(*target_properties);

      return pstring(p, source_name.to_string() + hash_suffix + *target_type.suffixes().begin());
   }
}

pstring cut_suffix(pool& p, const pstring& source_name,
                   const type& source_type)
{
   return pstring(p, std::string(source_name.begin(), source_name.begin() + (source_name.size() - source_type.suffix_for(source_name.to_string()).size()))); // FIXME: Here is conversion from pstring to string
}

}
