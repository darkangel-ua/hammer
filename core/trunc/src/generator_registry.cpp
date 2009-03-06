#include "stdafx.h"
#include <hammer/core/generator_registry.h>
#include <stdexcept>
#include <hammer/core/main_target.h>
#include <hammer/core/type.h>
#include <boost/format.hpp>
#include <hammer/core/feature_set.h>

using namespace std;
using namespace boost;

namespace hammer{

void generator_registry::insert(std::auto_ptr<generator> g)
{
   generator* tmp = g.get();
   if (!generators_.insert(tmp->name(), g).second)
      throw std::runtime_error("Generator '" + g->name() + "' already registered.");
}

static int compute_rank(const feature_set& build_properties, const feature_set& constraints)
{
   int rank = 0;
   for(feature_set::const_iterator i = constraints.begin(), last = constraints.end(); i != last; ++i)
      if (build_properties.find(**i) != build_properties.end())
         ++rank;
      else 
         return -1;

   return rank;
}

generator_registry::viable_generators_t 
generator_registry::find_viable_generators(const type& t, 
                                           bool allow_composite,
                                           const feature_set& build_properties) const
{
   viable_generators_t result;
   int rank = 0; // rank show as the weight of generator (the more rank the more generator suitable for generation this type of targets)
   for(generators_t::const_iterator i = generators_.begin(), last = generators_.end(); i != last; ++i)
   {
      if (i->second->is_composite() && !allow_composite ||
          !i->second->is_composite() && allow_composite)
      {
         continue;
      }

      for(generator::producable_types_t::const_iterator j = i->second->producable_types().begin(), j_last = i->second->producable_types().end(); j != j_last; ++j)
      {
         if (t.equal_or_derived_from(*j->type_))
         {
            int generator_rank = i->second->constraints() != NULL ? compute_rank(build_properties, *i->second->constraints())
                                                                  : 0;
            if (generator_rank == -1) // build properties not satisfied generator constraints
               continue;

            if (generator_rank > rank)
            {
               rank = generator_rank;
               result.clear();
            }
            
            if (rank == generator_rank)
               result.push_back(i->second);  
         }
      }
   }

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
      vector<const generator*> vg(find_viable_generators(*i->type_, current_generator.include_composite_generators(), props));
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

namespace
{
   struct generator_data
   {
      generator_data(const generator* g) : generator_(g), all_consumed_(true) {} 

      const generator* generator_;
      vector<intrusive_ptr<build_node> > transformed_sources_;
      bool all_consumed_;
   };
}

std::vector<boost::intrusive_ptr<build_node> >
generator_registry::construct(const main_target* mt) const
{
   typedef std::vector<generator_data> main_viable_generators_t;
   viable_generators_t viable_generators(find_viable_generators(mt->type(), true, mt->properties()));
   main_viable_generators_t main_viable_generators(viable_generators.begin(), viable_generators.end());

   if (main_viable_generators.empty())
      throw runtime_error("Can't find transformation to '" + mt->type().tag().name() + "'.");

   // generate target sources
   vector<intrusive_ptr<build_node> > generated_sources;
   for(main_target::sources_t::const_iterator i = mt->sources().begin(), last = mt->sources().end(); i != last; ++i)
   {
      std::vector<boost::intrusive_ptr<build_node> > r((**i).generate());
      generated_sources.insert(generated_sources.end(), r.begin(), r.end());
   }

   // transform all sources using all viable generators
   while(!generated_sources.empty())
   {
      intrusive_ptr<build_node> s(generated_sources.back());
      generated_sources.pop_back();
      for(main_viable_generators_t::iterator i = main_viable_generators.begin(), last = main_viable_generators.end(); i != last; ++i)
         i->all_consumed_= i->all_consumed_ && transform_to_consumable(*i->generator_, *i->generator_, s, &i->transformed_sources_, mt->properties(), *mt);
   }

   // search for ONE good generator that consume all sources
   bool has_choosed_generator = false;
   main_viable_generators_t::const_iterator choosed_generator;
   for(main_viable_generators_t::const_iterator i = main_viable_generators.begin(), last = main_viable_generators.end(); i != last; ++i)
   {
      // FIXME: error messages
      if (has_choosed_generator && i->all_consumed_)
         throw runtime_error("Found more than one transformations from sources to target.");
      
      if (i->all_consumed_)
      {
         choosed_generator = i;
         has_choosed_generator = true;
      }
   }

   // FIXME: error messages
   if (!has_choosed_generator)
      throw runtime_error((boost::format("Can't find transformation 'sources' -> '%s'.") 
                              % mt->type().tag().name()).str());

   std::vector<boost::intrusive_ptr<build_node> > r(choosed_generator->generator_->construct(mt->type(), mt->properties(), choosed_generator->transformed_sources_, 0, &mt->name(), *mt));
   if (!r.empty())
      return r;

   // FIXME: error messages
   throw std::runtime_error("No viable generator found.");
}

}