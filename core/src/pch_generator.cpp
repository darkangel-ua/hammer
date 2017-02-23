#include "stdafx.h"
#include <hammer/core/pch_generator.h>
#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/target_type.h> 

namespace hammer
{

pch_generator::pch_generator(hammer::engine& e,
                             const std::string& name,
                             const consumable_types_t& source_types,
                             const producable_types_t& target_types,
                             bool composite,
                             const feature_set* c)
   : generator(e, name, source_types, target_types, composite, c),
     c_type_(e.get_type_registry().get(types::C)),
     cpp_type_(e.get_type_registry().get(types::CPP)),
     h_type_(e.get_type_registry().get(types::H))
{
}

build_nodes_t
pch_generator::construct(const target_type& type_to_construct, 
                         const feature_set& props,
                         const std::vector<boost::intrusive_ptr<build_node> >& sources,
                         const basic_target* t,
                         const std::string* composite_target_name,
                         const main_target& owner) const
{
   // leave only CPP C H targets - discard others
   typedef std::vector<boost::intrusive_ptr<build_node> > sources_t;
   sources_t modified_sources;
   for(sources_t::const_iterator i = sources.begin(), last = sources.end(); i != last; ++i)
   {
      const target_type* t = (**i).targeting_type_;
      if (t->equal_or_derived_from(c_type_) ||
          t->equal_or_derived_from(cpp_type_) ||
          t->equal_or_derived_from(h_type_))
      {
         modified_sources.push_back(*i);
      }
   }

   return generator::construct(type_to_construct, props, modified_sources, t, composite_target_name, owner);
}

}
