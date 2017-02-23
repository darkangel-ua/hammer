#include "stdafx.h"
#include <hammer/core/fake_generator.h>
#include <hammer/core/fake_target.h>

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

build_nodes_t
fake_generator::construct(const target_type& type_to_construct, 
                          const feature_set& props,
                          const build_nodes_t& sources,
                          const basic_target* source_target,
                          const std::string* composite_target_name,
                          const main_target& owner) const
{
   // Because fake target always exists and up to date we need make it depends on sources
   // so actuality checker will go thought it
   
   build_nodes_t result = generator::construct(type_to_construct, props, sources, source_target, composite_target_name, owner);

   build_nodes_t dependencies;
   for(const build_nodes_t::value_type& n : result)
      for(const build_node::source_t& s : n->sources_)
         dependencies.push_back(s.source_node_);

   remove_dups(dependencies);

   for(const build_nodes_t::value_type& n : result)
      n->dependencies_.insert(n->dependencies_.end(), dependencies.begin(), dependencies.end());

   return result;
}

basic_target* fake_generator::create_target(const main_target* mt,
                                            const build_node::sources_t& sources,
                                            const std::string& n,
                                            const target_type* t,
                                            const feature_set* f) const
{
   return new fake_target(mt, sources, n, t, f);
}

}
