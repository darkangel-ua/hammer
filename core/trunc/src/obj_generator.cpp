#include "stdafx.h"
#include <hammer/core/obj_generator.h>
#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/type.h>

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

std::vector<boost::intrusive_ptr<build_node> >
obj_generator::construct(const type& target_type, 
                         const feature_set& props,
                         const std::vector<boost::intrusive_ptr<build_node> >& sources,
                         const basic_target* t,
                         const pstring* composite_target_name,
                         const main_target& owner) const
{
   boost::intrusive_ptr<build_node> result(new build_node);
   result->down_ = sources;
   result->targeting_type_ = &obj_type_;

   // discard all other types except OBJ and 
   // FIXME: this snipped copy pasted from generator.cpp. Need to refactor in more generic way
   typedef std::vector<boost::intrusive_ptr<build_node> >::const_iterator iter;
   for(iter i = sources.begin(), last = sources.end(); i != last; ++i)
   {
      bool node_added = false;
      for(build_node::targets_t::const_iterator p_i = (**i).products_.begin(), p_last = (**i).products_.end(); p_i != p_last; ++p_i)
      {
         if ((**p_i).type().equal_or_derived_from(obj_type_))
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
   
   return std::vector<boost::intrusive_ptr<build_node> >(1, result);
}

}