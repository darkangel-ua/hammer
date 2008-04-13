#include "stdafx.h"
#include "generator_registry.h"
#include <stdexcept>
#include "main_target.h"
#include "type.h"

using namespace std;

namespace hammer{

void generator_registry::insert(const generator& g)
{
   if (!generators_.insert(std::make_pair(g.name(), g)).second)
      throw std::runtime_error("Generator '" + g.name() + "' already registered.");
}

std::vector<const generator*> 
generator_registry::find_viable_generators(const type& t) const
{
   vector<const generator*> result;
   for(generators_t::const_iterator i = generators_.begin(), last = generators_.end(); i != last; ++i)
   {
      for(generator::producable_types_t::const_iterator j = i->second.producable_types().begin(), j_last = i->second.producable_types().end(); j != j_last; ++j)
      {
         if (*j->type_ == t)
            result.push_back(&i->second);
      }
   }

   if (result.empty())
      throw std::runtime_error("No generator found");
   
   if (result.size() != 1)
      throw std::runtime_error("More than one generator found");

   return result;
}

std::auto_ptr<build_node> 
generator_registry::construct(main_target* mt) const
{
   vector<const generator*> viable_generators(find_viable_generators(mt->type()));
   boost::ptr_vector<build_node> sources;
   for(main_target::sources_t::const_iterator i = mt->sources().begin(), last = mt->sources().end(); i != last; ++i)
   {
      std::auto_ptr<build_node> r((**i).generate());
      sources.push_back(r.get());
      r.release();
   }

   typedef vector<const generator*>::const_iterator iter;
   for(iter i = viable_generators.begin(), last = viable_generators.end(); i != last; ++i)
   {
      std::auto_ptr<build_node> r((*i)->construct(mt->type(), mt->features(), sources));
      if (r.get())
         return r;
   }

   throw std::runtime_error("No viable generator found.");
}

}