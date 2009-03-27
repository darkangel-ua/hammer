#include "stdafx.h"
#include <hammer/core/obj_generator.h>
#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/target_type.h>

namespace hammer
{

obj_generator::obj_generator(hammer::engine& e,
                             const std::string& name,
                             const consumable_types_t& source_types,
                             const producable_types_t& target_types,
                             bool composite,
                             const feature_set* c)
   : generator(e, name, source_types, target_types, composite, c),
     obj_type_(e.get_type_registry().get(types::OBJ))
{

}

// Move to result only OBJ targets and skip any others
std::vector<boost::intrusive_ptr<build_node> >
obj_generator::construct(const target_type& type_to_construct, 
                         const feature_set& props,
                         const std::vector<boost::intrusive_ptr<build_node> >& sources,
                         const basic_target* t,
                         const pstring* composite_target_name,
                         const main_target& owner) const
{
   typedef std::vector<boost::intrusive_ptr<build_node> > build_sources_t;

   std::vector<boost::intrusive_ptr<build_node> > result;

   for(build_sources_t::const_iterator i = sources.begin(); i != sources.end(); ++i)
   {
      if ((**i).targeting_type_->equal_or_derived_from(obj_type_))
         result.push_back(*i);
   }

   return result;
}

}