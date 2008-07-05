#include "stdafx.h"
#include "np_helpers.h"
#include "pstring.h"
#include "type.h"

namespace hammer{

pstring make_name(pool& p, const pstring& source_name, 
                  const type& source_type, const type& target_type)
{
   return pstring(p, std::string(source_name.begin(), source_name.begin() + (source_name.size() - source_type.suffix().size())) + target_type.suffix());
}

pstring make_name(pool& p, const pstring& source_name, 
                  const type& target_type)
{
   return pstring(p, source_name.to_string() + target_type.suffix());
}

pstring cut_suffix(pool& p, const pstring& source_name,
                   const type& source_type)
{
   return pstring(p, std::string(source_name.begin(), source_name.begin() + (source_name.size() - source_type.suffix().size())));
}

}
