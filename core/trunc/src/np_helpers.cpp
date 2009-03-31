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

pstring make_product_name(pool& p, 
                          const basic_target& source_target,
                          const target_type& product_type,
                          const feature_set& product_properties,
                          const main_target* owner)
{
   string source_name = source_target.name().to_string();
   const std::string& source_suffix = source_target.type().suffix_for(source_name, source_target.properties());
   // find leaf in source name to leave only filename
   string::size_type slash_pos = source_name.find_last_of('/');
   if (slash_pos == string::npos)
      slash_pos = 0;

   std::string hash_suffix;
   if (owner != NULL)
   {
      string version = get_version(product_properties);
      if (!version.empty())
         hash_suffix = '-' + version;

      hash_suffix += '-' + compute_hash(product_properties, *owner);
   }
 
   string source_name_without_suffix = std::string(source_name.begin() + slash_pos, 
                                                   source_name.begin() + (source_name.size() - source_suffix.size()));
   return pstring(p, source_name_without_suffix + hash_suffix + product_type.suffix_for(product_properties));
}

pstring make_product_name(pool& p, 
                          const pstring& composite_target_name, 
                          const target_type& product_type,
                          const feature_set& product_properties,
                          const main_target* owner)
{
   if (product_type.suffixes().empty())
      return composite_target_name;
   else
   {
      std::string hash_suffix;
      if (owner != NULL)
      {
         string version = get_version(product_properties);
         if (!version.empty())
            hash_suffix = '-' + version;

         hash_suffix += '-' + compute_hash(product_properties, *owner);
      }

      return pstring(p, composite_target_name.to_string() + hash_suffix + product_type.suffix_for(product_properties));
   }
}

}
