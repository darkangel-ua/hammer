#include "stdafx.h"
#include <boost/assign/list_of.hpp>
#include <hammer/core/obj_generator.h>
#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/target_type.h>

using namespace boost::assign;

namespace hammer
{

obj_generator::obj_generator(hammer::engine& e)
   : generator(e,
               "obj meta target generator",
               list_of<consumable_type>(e.get_type_registry().get(types::OBJ))
                                       (e.get_type_registry().get(types::LIB))
                                       (e.get_type_registry().get(types::H)),
               list_of<produced_type>(e.get_type_registry().get(types::OBJ)),
               true),
     obj_type_(e.get_type_registry().get(types::OBJ))
{
}

// Move to result only OBJ targets and skip any others
build_nodes_t
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
