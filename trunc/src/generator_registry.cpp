#include "stdafx.h"
#include "generator_registry.h"
#include <stdexcept>

using namespace std;

namespace hammer{

void generator_registry::insert(const generator& g)
{
   if (!generators_.insert(std::make_pair(g.name(), g)).second)
      throw std::runtime_error("Generator '" + g.name() + "' already registered.");
}

vector<basic_target*> 
generator_registry::transform(const vector<basic_target*>& targets) const
{
   vector<basic_target*> result;
   /*
   size_t idx = 0;
   int max_consumed = -1;

   for(generators_t::const_iterator i = generators_.begin(), last = generators_.end(); i != last; ++i, ++idx)
   {
      int consumed = i->can_consume(targets);
      if (max_consumed < consumed)
   }
*/

   return result;
}

}