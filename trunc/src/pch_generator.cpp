#include "stdafx.h"
#include "pch_generator.h"
#include "types.h"
#include "engine.h"

namespace hammer
{

 pch_generator::pch_generator(hammer::engine& e,
                             const std::string& name,
                             const consumable_types_t& source_types,
                             const producable_types_t& target_types,
                             bool composite,
                             const feature_set* c)
   : generator(e, name, source_types, target_types, composite, c),
     c_type_(&e.get_type_registry().resolve_from_name(types::C)),
     cpp_type_(&e.get_type_registry().resolve_from_name(types::CPP)),
     h_type_(&e.get_type_registry().resolve_from_name(types::H))
{
}

std::vector<boost::intrusive_ptr<build_node> >
pch_generator::construct(const type& target_type, 
                         const feature_set& props,
                         const std::vector<boost::intrusive_ptr<build_node> >& sources,
                         const basic_target* t,
                         const pstring* composite_target_name,
                         const main_target& owner) const
{
   // leave only CPP C H target - discard others
   typedef std::vector<boost::intrusive_ptr<build_node> > sources_t;
   sources_t modified_sources;
   for(sources_t::const_iterator i = sources.begin(), last = sources.end(); i != last; ++i)
   {
      const type* t = (**i).targeting_type_;
      if (t == c_type_ ||
          t == cpp_type_ ||
          t == h_type_)
      {
         modified_sources.push_back(*i);
      }
   }

   return generator::construct(target_type, props, modified_sources, t, composite_target_name, owner);
}

}