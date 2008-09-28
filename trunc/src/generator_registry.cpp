#include "stdafx.h"
#include "generator_registry.h"
#include <stdexcept>
#include "main_target.h"
#include "type.h"

using namespace std;
using namespace boost;

namespace hammer{

void generator_registry::insert(std::auto_ptr<generator> g)
{
   generator* tmp = g.get();
   if (!generators_.insert(tmp->name(), g).second)
      throw std::runtime_error("Generator '" + g->name() + "' already registered.");
}

vector<const generator*> 
generator_registry::find_viable_generators(const type& t, bool allow_composite) const
{
   vector<const generator*> result;
   for(generators_t::const_iterator i = generators_.begin(), last = generators_.end(); i != last; ++i)
   {
      if (i->second->is_composite() && !allow_composite)
         continue;

      for(generator::producable_types_t::const_iterator j = i->second->producable_types().begin(), j_last = i->second->producable_types().end(); j != j_last; ++j)
      {
         if (*j->type_ == t)
            result.push_back(i->second);
      }
   }

//   disable this check until more complicated algorithm will be introduced
//   if (result.size() > 1)
//      throw std::runtime_error("More than one generator found");

   return result;
}

bool
generator_registry::transform(const generator& target_generator, 
                              const generator& current_generator, 
                              const basic_target* t,
                              boost::intrusive_ptr<build_node> target_owner, 
                              std::vector<boost::intrusive_ptr<build_node> >* result,
                              const feature_set& props,
                              const main_target& owner) const
{
   for(generator::consumable_types_t::const_iterator i = current_generator.consumable_types().begin(), last = current_generator.consumable_types().end(); i != last; ++i)
   {
      vector<const generator*> vg(find_viable_generators(*i->type_, false));
      if (vg.empty())
         return false;

      for(vector<const generator*>::const_iterator g_i = vg.begin(), g_last = vg.end(); g_i != g_last; ++g_i)
      {
         if ((**g_i).is_consumable(t->type()))
         {
            std::vector<boost::intrusive_ptr<build_node> > r((**g_i).construct(*i->type_, props, vector<intrusive_ptr<build_node> >(1, target_owner), t, 0, owner));
            result->insert(result->end(), r.begin(), r.end());
            return true;
         }
         else
         {
            vector<intrusive_ptr<build_node> > this_result;
            if (transform(target_generator, **g_i, t, target_owner, &this_result, props, owner))
            {
               for(vector<intrusive_ptr<build_node> >::const_iterator r_i = this_result.begin(), r_last = this_result.end(); r_i != r_last; ++r_i)
                  transform_to_consumable(target_generator, current_generator, *r_i, result, props, owner);
               
               return true;
            }
         }
      }
   }

   return false;
}

bool generator_registry::transform_to_consumable(const generator& target_generator, 
                                                 const generator& current_generator,
                                                 intrusive_ptr<build_node> t, 
                                                 vector<intrusive_ptr<build_node> >* result,
                                                 const feature_set& props,
                                                 const main_target& owner) const
{
   bool some_was_consumed = false;
   bool has_direct_consumable = false;
   for(build_node::targets_t::const_iterator i = t->products_.begin(), last = t->products_.end(); i != last; ++i)
   {
      if (target_generator.is_consumable((**i).type()) ||
          current_generator.is_consumable((**i).type()))
      {
         if (!has_direct_consumable)
         {
            result->push_back(t);
            has_direct_consumable = true;
            some_was_consumed = true;
         }
      }
      else
      {
         bool res = transform(target_generator, current_generator, *i, t, result, props, owner);
         
         if (res)
            some_was_consumed = true;
      }
   }

   return some_was_consumed;
}

std::vector<boost::intrusive_ptr<build_node> >
generator_registry::construct(main_target* mt) const
{
   vector<const generator*> viable_generators(find_viable_generators(mt->type(), true));
   vector<intrusive_ptr<build_node> > pre_sources;
   for(main_target::sources_t::const_iterator i = mt->sources().begin(), last = mt->sources().end(); i != last; ++i)
   {
      std::vector<boost::intrusive_ptr<build_node> > r((**i).generate());
      pre_sources.insert(pre_sources.end(), r.begin(), r.end());
   }

   vector<intrusive_ptr<build_node> > sources;
   while(!pre_sources.empty())
   {
      intrusive_ptr<build_node> s(pre_sources.back());
      pre_sources.pop_back();
      if (!transform_to_consumable(*viable_generators[0], *viable_generators[0], s, &sources, mt->properties(), *mt))
         throw runtime_error("Can't find transformation from '?' -> '" + mt->type().name() + "'.");
   }

   typedef vector<const generator*>::const_iterator iter;
   for(iter i = viable_generators.begin(), last = viable_generators.end(); i != last; ++i)
   {
      std::vector<boost::intrusive_ptr<build_node> > r((*i)->construct(mt->type(), mt->properties(), sources, 0, &mt->name(), *mt));
      if (!r.empty())
         return r;
   }

   throw std::runtime_error("No viable generator found.");
}

}