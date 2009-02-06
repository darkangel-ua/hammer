#include "stdafx.h"
#include "call_resolver.h"

namespace hammer{

std::auto_ptr<call_resolver_call_arg_base> 
call_resolver::invoke(const char* func_name, args_list_t& args) 
{ 
   functions_t::const_iterator i = find(func_name);
   if (i == functions_.end())
      throw std::runtime_error("Unknown function '" + std::string(func_name) + "'");

   return i->second->invoke(args); 
}

call_resolver::const_iterator call_resolver::find(const char* func_name) const
{
   return functions_.find(func_name);
}


}