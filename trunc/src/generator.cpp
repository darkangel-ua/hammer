#include "stdafx.h"
#include "generator.h"

namespace hammer{

generator::generator(const std::string& name,
                     const consumable_types_t& source_types,
                     const producable_types_t& target_types,
                     const feature_set* c) : 
   name_(name), source_types_(source_types_),
   target_types_(target_types),
   constraints_(c)
{

}
 
std::auto_ptr<build_node>
generator::construct(const type& target_type, 
                     const feature_set& props,
                     const boost::ptr_vector<build_node>& sources) const
{
   return std::auto_ptr<build_node>();
}

}