#include "stdafx.h"
#include "np_helpers.h"
#include "pstring.h"
#include "type.h"

namespace hammer{

pstring make_name(pool& p, const pstring& source_name, 
                  const type& source_type, const type& target_type)
{
   const std::string& source_suffix = source_type.suffix_for(source_name.to_string()); // FIXME: Here is conversion from pstring to string
   return pstring(p, std::string(source_name.begin(), source_name.begin() + (source_name.size() - source_suffix.size())) + *target_type.suffixes().begin());
}

pstring make_name(pool& p, const pstring& source_name, 
                  const type& target_type)
{
   if (target_type.suffixes().empty())
      return source_name;
   else
      return pstring(p, source_name.to_string() + *target_type.suffixes().begin());
}

pstring cut_suffix(pool& p, const pstring& source_name,
                   const type& source_type)
{
   return pstring(p, std::string(source_name.begin(), source_name.begin() + (source_name.size() - source_type.suffix_for(source_name.to_string()).size()))); // FIXME: Here is conversion from pstring to string
}

}
