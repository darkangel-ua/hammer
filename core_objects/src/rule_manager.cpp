#include <hammer/core/rule_manager.h>

namespace hammer{

rule_declaration::const_iterator 
rule_declaration::find(const parscore::identifier& arg_name) const
{
   for(const_iterator i = begin(), last = end(); i != last; ++i)
      if (i->name() == arg_name)
         return i;

   return end();
}

rule_manager_arg_ptr
rule_declaration::invoke(rule_manager_arguments_t& arguments) const
{
   return invoker_->invoke(arguments);
}

}
