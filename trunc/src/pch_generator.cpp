#include "stdafx.h"
#include "pch_generator.h"

namespace hammer
{

pch_generator::pch_generator(engine& e,
                             const std::string& name,
                             const consumable_types_t& source_types,
                             const producable_types_t& target_types,
                             bool composite,
                             const feature_set* c)
   : generator(e, name, source_types, target_types, composite, c)
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

}

}