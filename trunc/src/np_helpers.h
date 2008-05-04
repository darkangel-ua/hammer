#pragma once

namespace hammer
{
   class type;   
   class pstring;
   class pool;

   pstring make_name(pool& p, const pstring& source_name, 
                     const type& source_type, const type& target_type);
   pstring make_name(pool& p, const pstring& source_name, 
                     const type& target_type);

}