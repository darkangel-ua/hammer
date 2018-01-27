#include "stdafx.h"
#include <hammer/core/np_helpers.h>
#include <hammer/core/target_type.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/basic_build_target.h>
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
      return (**i).value();
   else
      return string();
}

std::string
make_product_name(const basic_build_target& source_target,
                  const target_type& product_type,
                  const feature_set& product_properties,
                  const main_target* owner)
{
   const string& source_name = source_target.name();
   const std::string& source_suffix = source_target.type().suffix_for(source_name, source_target.properties());

   std::string hash_suffix;
   if (owner != NULL)
   {
      string version = get_version(product_properties);
      if (!version.empty())
         hash_suffix = '-' + version;

      hash_suffix += '-' + md5(product_properties, true);
   }
 
   string source_name_without_suffix = std::string(source_name.begin(),
                                                   source_name.begin() + (source_name.size() - source_suffix.size()));
   return product_type.prefix_for(product_properties) + source_name_without_suffix + hash_suffix + product_type.suffix_for(product_properties);
}

std::string
make_product_name(const std::string& composite_target_name,
                  const target_type& product_type,
                  const feature_set& product_properties,
                  const main_target* owner,
                  bool primary_target)
{
   feature_set::const_iterator n = product_properties.find("name");
   if (n != product_properties.end() && primary_target)
      return (**n).value();

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

         hash_suffix += '-' + md5(product_properties, true);
      }

      return product_type.prefix_for(product_properties) + composite_target_name + hash_suffix + product_type.suffix_for(product_properties);
   }
}

}
