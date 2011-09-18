#include "stdafx.h"
#include <hammer/core/fake_generator.h>
#include <hammer/core/fake_target.h>
#include <boost/foreach.hpp>

namespace hammer{

fake_generator::fake_generator(engine& e,
                               const std::string& name,
                               const consumable_types_t& source_types,
                               const producable_types_t& target_types,
                               bool composite,
                               const feature_set* c)
   : generator(e, name, source_types, target_types, composite, c)
{
}

generator::construct_result_t
fake_generator::construct(const target_type& type_to_construct, 
                          const feature_set& props,
                          const build_nodes_t& sources,
                          const basic_target* source_target,
                          const pstring* composite_target_name,
                          const main_target& owner) const
{
   // Because fake target always exists and up to date we need make it depends on sources
   // so actuality checker will go thought it
   
   construct_result_t result = generator::construct(type_to_construct, props, sources, source_target, composite_target_name, owner);

   build_nodes_t dependencies;
   BOOST_FOREACH(const build_nodes_t::value_type& n, result.result_)
      BOOST_FOREACH(const build_node::source_t& s, n->sources_)
         dependencies.push_back(s.source_node_);

   result.dependencies_.insert(result.dependencies_.end(), dependencies.begin(), dependencies.end());
   remove_dups(result.dependencies_);

   BOOST_FOREACH(const build_nodes_t::value_type& n, result.result_)
      n->dependencies_.insert(n->dependencies_.end(), result.dependencies_.begin(), result.dependencies_.end());

   result.dependencies_.clear();

   return result;
}

basic_target* fake_generator::create_target(const main_target* mt, 
                                            const pstring& n, 
                                            const target_type* t, 
                                            const feature_set* f) const
{
   return new fake_target(mt, n, t, f);
}

}
