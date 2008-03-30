#include "stdafx.h"
#include "generator_registry.h"
#include <stdexcept>
#include "main_target.h"

using namespace std;

namespace hammer{

void generator_registry::insert(const generator& g)
{
   if (!generators_.insert(std::make_pair(g.name(), g)).second)
      throw std::runtime_error("Generator '" + g.name() + "' already registered.");
}

vector<basic_target*> 
generator_registry::transform(main_target* mt) const
{
   vector<basic_target*> result;
   vector<basic_target*> from_targets(mt->sources());
   for(generators_t::const_iterator i = generators_.begin(), last = generators_.end(); i != last; ++i)
   {
      vector<basic_target*> r = i->second.transform(mt, from_targets);
   }

   return result;
}

}