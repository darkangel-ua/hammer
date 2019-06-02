#include <hammer/core/generator_registry.h>
#include <stdexcept>
#include <hammer/core/main_target.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/target_type.h>
#include <boost/format.hpp>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/subfeature.h>

using namespace std;
using namespace boost;

namespace hammer {

void generator_registry::insert(std::unique_ptr<generator> g)
{
   std::string g_name = g->name();
   if (!generators_.emplace(g_name, std::move(g)).second)
      throw std::runtime_error("Generator '" + g_name + "' already registered.");
}

// build request: <toolset>gcc
// constraints:   <toolset>gcc-6
// should fail because constraints is more specific
// and for
// build request: <toolset>gcc-6
// constraints:   <toolset>gcc
// should match because constraints is wider than build request

static
int compute_rank(const feature& from_build_request,
                 const feature& from_constraints)
{
   if (from_build_request.value() != from_constraints.value())
      return -1;

   int rank = 100;

   auto c_first = from_constraints.subfeatures().begin();
   auto c_last = from_constraints.subfeatures().end();

   for (; c_first != c_last; ++c_first) {
      const subfeature* cs = from_build_request.find_subfeature((**c_first).name());
      if (!cs || cs->value() != (**c_first).value())
         return -1;

      rank += 1;
   }

   return rank;
}

static
int compute_rank(const feature_set& build_properties,
                 const feature_set& constraints)
{
   int rank = 0;
   for(feature_set::const_iterator i = constraints.begin(), last = constraints.end(); i != last; ++i) {
      auto i_in_build = build_properties.find((**i).name());
      if (i_in_build != build_properties.end()) {
         const int f_runk = compute_rank(**i_in_build, **i);
         if (f_runk == -1)
            return -1;

         rank += f_runk;
      } else
         return -1;
   }

   return rank;
}

generator_registry::viable_generators_t
generator_registry::find_viable_generators(const target_type& t,
                                           bool allow_composite,
                                           const feature_set& build_properties) const
{
   viable_generators_t result = find_viable_generators(t, allow_composite, build_properties, /*full_match=*/true, nullptr);
   if (!result.empty())
      return result;
   else
      return find_viable_generators(t, allow_composite, build_properties, /*full_match=*/false, nullptr);
}

generator_registry::viable_generators_t 
generator_registry::find_viable_generators(const target_type& t, 
                                           bool allow_composite,
                                           const feature_set& build_properties,
                                           bool full_match,
                                           const generator* excluded) const
{
   viable_generators_t result;
   int rank = 0; // rank show as the weight of generator (the more rank the more generator suitable for generation this type of targets)
   for (auto& ng : generators_) {
      const generator& g = *ng.second;
      if ((g.is_composite() && !allow_composite) ||
          (!g.is_composite() && allow_composite) ||
          &g == excluded)
      {
         continue;
      }

      for (generator::producable_types_t::const_iterator j = g.producable_types().begin(), j_last = g.producable_types().end(); j != j_last; ++j) {
         if ((!full_match && j->type_->equal_or_derived_from(t)) ||
             (full_match && *j->type_ == t))
         {
            int generator_rank = g.constraints() ? compute_rank(build_properties, *g.constraints()) : 0;
            if (generator_rank == -1) // build properties not satisfied generator constraints
               continue;

            if (generator_rank > rank) {
               rank = generator_rank;
               result.clear();
            }

            if (rank == generator_rank)
               result.push_back(make_pair(&g, j->type_));
         }
      }
   }

   return result;

}

bool
generator_registry::transform(const generator& target_generator,
                              const generator& current_generator,
                              const basic_build_target* source_target,
                              build_node_ptr& source_node,
                              build_nodes_t* result,
                              const feature_set& props,
                              const main_target& owner) const
{
   for(generator::consumable_types_t::const_iterator i = current_generator.consumable_types().begin(), last = current_generator.consumable_types().end(); i != last; ++i)
   {
      viable_generators_t vg = find_viable_generators(*i->type_, current_generator.include_composite_generators(), props, /*full_match=*/true, &current_generator);
      if (vg.empty())
         continue;

      for(viable_generators_t::const_iterator g_i = vg.begin(), g_last = vg.end(); g_i != g_last; ++g_i)
      {
         if (g_i->first->is_consumable(source_target->type()))
         {
            build_nodes_t r(g_i->first->construct(*g_i->second, props, build_nodes_t(1, source_node), source_target, 0, owner));
            remove_dups(source_node->dependencies_);
            result->insert(result->end(), r.begin(), r.end());
            return true;
         }
         else
         {
            build_nodes_t this_result;
            if (transform(target_generator, *g_i->first, source_target, source_node, &this_result, props, owner))
            {
               for(build_nodes_t::const_iterator r_i = this_result.begin(), r_last = this_result.end(); r_i != r_last; ++r_i)
                  transform_to_consumable(target_generator, current_generator, *r_i, result, props, owner);

               return true;
            }
         }
      }

      // !!!!! This is copy paste from above!!!!
      vg = find_viable_generators(*i->type_, current_generator.include_composite_generators(), props, /*full_match=*/false, &current_generator);
      if (vg.empty())
         continue;

      for(viable_generators_t::const_iterator g_i = vg.begin(), g_last = vg.end(); g_i != g_last; ++g_i)
      {
         if (g_i->first->is_consumable(source_target->type()))
         {
            build_nodes_t r(g_i->first->construct(*g_i->second, props, build_nodes_t(1, source_node), source_target, 0, owner));
            remove_dups(source_node->dependencies_);
            result->insert(result->end(), r.begin(), r.end());
            return true;
         }
         else
         {
            build_nodes_t this_result;
            if (transform(target_generator, *g_i->first, source_target, source_node, &this_result, props, owner))
            {
               for(build_nodes_t::const_iterator r_i = this_result.begin(), r_last = this_result.end(); r_i != r_last; ++r_i)
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
                                                 build_node_ptr source_node,
                                                 build_nodes_t* result,
                                                 const feature_set& props,
                                                 const main_target& owner) const
{
   bool some_was_consumed = false;
   bool has_direct_consumable = false;
   for(build_node::targets_t::const_iterator i = source_node->products_.begin(), last = source_node->products_.end(); i != last; ++i)
   {
      if (target_generator.is_consumable((**i).type()) ||
          current_generator.is_consumable((**i).type()))
      {
         if (!has_direct_consumable)
         {
            result->push_back(source_node);
            has_direct_consumable = true;
            some_was_consumed = true;
         }
      }
      else
      {
         bool res = transform(target_generator, current_generator, *i, source_node, result, props, owner);

         if (res)
            some_was_consumed = true;
      }
   }

   return some_was_consumed;
}

namespace
{
   struct generator_data {
      generator_data(const generator* g)
         : generator_(g)
      {}

      const generator* generator_;
      build_nodes_t transformed_sources_;
      bool all_consumed_ = true;
   };
}

build_nodes_t
generator_registry::construct(const main_target* mt) const
{
   typedef std::vector<generator_data> main_viable_generators_t;
   viable_generators_t viable_generators(find_viable_generators(mt->type(), true, mt->properties()));
   main_viable_generators_t main_viable_generators;
   for(const viable_generators_t::value_type& g : viable_generators)
      main_viable_generators.push_back(g.first);

   if (main_viable_generators.empty())
      throw runtime_error("Can't find transformation to '" + mt->type().tag().name() + "'.");

   // generate target sources
   build_node::nodes_t generated_sources;
   for(main_target::sources_t::const_iterator i = mt->sources().begin(), last = mt->sources().end(); i != last; ++i)
   {
      build_nodes_t r((**i).generate());
      generated_sources.insert(generated_sources.end(), r.begin(), r.end());
   }

   // transform all sources using all viable generators
   for(build_node::nodes_t::const_iterator s = generated_sources.begin(), s_last = generated_sources.end(); s != s_last; ++s)
      for(main_viable_generators_t::iterator i = main_viable_generators.begin(), last = main_viable_generators.end(); i != last; ++i)
         i->all_consumed_= i->all_consumed_ && transform_to_consumable(*i->generator_, *i->generator_, *s, &i->transformed_sources_, mt->properties(), *mt);

   // search for ONE good generator that consume all sources
   bool has_choosed_generator = false;
   main_viable_generators_t::const_iterator choosed_generator;
   for(main_viable_generators_t::const_iterator i = main_viable_generators.begin(), last = main_viable_generators.end(); i != last; ++i)
   {
      // FIXME: error messages
      if (has_choosed_generator && i->all_consumed_)
         throw runtime_error("Found more than one transformations from sources to target.\n"
                             "First  was '" + choosed_generator->generator_->name() + "', constraints: " + dump_for_hash(*choosed_generator->generator_->constraints(), true) + "\n"
                             "Second was '" + i->generator_->name() + "', constraints: " + dump_for_hash(*i->generator_->constraints(), true));

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

   return choosed_generator->generator_->construct(mt->type(), mt->properties(), choosed_generator->transformed_sources_, 0, &mt->name(), *mt);
}

}
