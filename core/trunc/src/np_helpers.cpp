#include "stdafx.h"
#include <hammer/core/np_helpers.h>
#include <hammer/core/pstring.h>
#include <hammer/core/target_type.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/basic_target.h>
#include <boost/crypto/md5.hpp>

using namespace std;

namespace hammer{

static string compute_hash(const feature_set& features, const main_target& mt)
{
   return basic_target::hash_string(features, mt);
}

static string get_version(const feature_set& properties)
{
   feature_set::const_iterator i = properties.find("version");
   if (i != properties.end())
      return (**i).value().to_string();
   else
      return string();
}

pstring make_name(pool& p, 
                  const pstring& source_name, 
                  const target_type& source_type, 
                  const target_type& type,
                  const feature_set* target_properties,
                  const main_target* owner)
{
   const std::string& source_suffix = source_type.suffix_for(source_name.to_string()); // FIXME: Here is conversion from pstring to string
   // find leaf in source name to leave only filename
   string::size_type slash_pos = source_name.find_last_of('/');
   if (slash_pos == string::npos)
      slash_pos = 0;

   std::string hash_suffix;
   if (target_properties != NULL)
   {
      string version = get_version(*target_properties);
      if (!version.empty())
         hash_suffix = '-' + version;

      hash_suffix += '-' + compute_hash(*target_properties, *owner);
   }
 
   return pstring(p, std::string(source_name.begin() + slash_pos, source_name.begin() + (source_name.size() - source_suffix.size())) + hash_suffix + *type.suffixes().begin());
}

pstring make_name(pool& p, 
                  const pstring& source_name, 
                  const target_type& target_type,
                  const feature_set* target_properties,
                  const main_target* owner)
{
   if (target_type.suffixes().empty())
      return source_name;
   else
   {
      std::string hash_suffix;
      if (target_properties != NULL)
      {
         string version = get_version(*target_properties);
         if (!version.empty())
            hash_suffix = '-' + version;

         hash_suffix += '-' + compute_hash(*target_properties, *owner);
      }

      return pstring(p, source_name.to_string() + hash_suffix + *target_type.suffixes().begin());
   }
}

pstring cut_suffix(pool& p, const pstring& source_name,
                   const target_type& source_type)
{
   return pstring(p, std::string(source_name.begin(), source_name.begin() + (source_name.size() - source_type.suffix_for(source_name.to_string()).size()))); // FIXME: Here is conversion from pstring to string
}

}
