#include "stdafx.h"
#include <hammer/core/generator.h>
#include <hammer/core/target_type.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/generated_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/np_helpers.h>
#include <hammer/core/build_action.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>

namespace hammer{

generator::generator(hammer::engine& e,
                     const std::string& name,
                     const consumable_types_t& source_types,
                     const producable_types_t& target_types,
                     bool composite,
                     const feature_set* c) :
   engine_(&e), name_(name), source_types_(source_types),
   target_types_(target_types),
   composite_(composite),
   constraints_(c),
   include_composite_generators_(false)
{

}

bool generator::is_consumable(const target_type& t) const
{
   if (consumable_types().empty())
      return true;

   for(generator::consumable_types_t::const_iterator i = consumable_types().begin(), last = consumable_types().end(); i != last; ++i)
      if (i->type_->equal_or_derived_from(t))
         return true;

   return false;
}

std::vector<boost::intrusive_ptr<build_node> >
generator::construct(const target_type& type_to_construct,
                     const feature_set& props,
                     const std::vector<boost::intrusive_ptr<build_node> >& sources,
                     const basic_target* source_target,
                     const pstring* composite_target_name,
                     const main_target& owner) const
{
   if (!source_target)
   {
      boost::intrusive_ptr<build_node> result(new build_node(owner, composite_));
      result->action(action());

      typedef std::vector<boost::intrusive_ptr<build_node> >::const_iterator iter;
      for(iter i = sources.begin(), last = sources.end(); i != last; ++i)
      {
         bool node_added = false;
         for(build_node::targets_t::const_iterator p_i = (**i).products_.begin(), p_last = (**i).products_.end(); p_i != p_last; ++p_i)
         {
            if (is_consumable((**p_i).type()))
            {
               result->sources_.push_back(build_node::source_t(*p_i, *i));
               if (!node_added)
               {
                  result->down_.push_back(*i);
                  node_added = true;
               }
            }
         }
      }

      for(producable_types_t::const_iterator i = target_types_.begin(), last = target_types_.end(); i != last; ++i)
      {
         pstring new_name = make_product_name(engine_->pstring_pool(),
                                              *composite_target_name,
                                              *i->type_,
                                              props,
                                              i->need_tag_ ? &owner : NULL);
         result->products_.push_back(new generated_target(&owner,
                                                          new_name,
                                                          i->type_, &props));
      }

      result->targeting_type_ = &type_to_construct;
      return std::vector<boost::intrusive_ptr<build_node> >(1, result);
   }
   else
   {
      pstring new_name = make_product_name(engine_->pstring_pool(),
                                           *source_target,
                                           type_to_construct,
                                           props,
                                           producable_types().front().need_tag_ ? &owner : NULL);
      assert(sources.size() == 1);

      boost::intrusive_ptr<build_node> result(new build_node(owner, composite_));
      result->action(action());

      result->sources_.push_back(build_node::source_t(source_target, sources.front()));
      result->down_.push_back(sources.front());
      result->products_.push_back(new generated_target(&owner, new_name, producable_types().front().type_, &props));
      result->targeting_type_ = &type_to_construct;
      return std::vector<boost::intrusive_ptr<build_node> >(1, result);
  }
}

generator::~generator()
{

}

void remove_dups(build_node::nodes_t& nodes)
{
   using namespace boost::multi_index;
   typedef boost::intrusive_ptr<build_node> node_t;
   typedef multi_index_container<node_t, indexed_by<sequenced<>, ordered_unique<identity<node_t> > > > container_t;
   
   container_t c;
   container_t::nth_index<0>::type& idx = c.get<0>();
   for(build_node::nodes_t::const_reverse_iterator i = nodes.rbegin(), last = nodes.rend(); i != last; ++i)
      idx.push_back(*i);

   build_node::nodes_t result;
   result.reserve(c.get<0>().size());
   typedef container_t::nth_index<0>::type idx_0_t;
   for(idx_0_t::const_reverse_iterator i = c.get<0>().rbegin(), last = c.get<0>().rend(); i != last; ++i)
      result.push_back(*i);
   
   nodes.swap(result);
}

}
