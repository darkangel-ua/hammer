#include "stdafx.h"
#include <hammer/core/exe_and_shared_lib_generator.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/basic_build_target.h>

namespace hammer{

exe_and_shared_lib_generator::exe_and_shared_lib_generator(hammer::engine& e,
                                                           const std::string& name,
                                                           const consumable_types_t& source_types,
                                                           const producable_types_t& target_types,
                                                           bool composite,
                                                           const feature_set* c)
   : generator(e, name, source_types,
               target_types, composite, c),
     searched_lib_(e.get_type_registry().get(types::SEARCHED_LIB))
{
}

build_nodes_t
exe_and_shared_lib_generator::construct(const target_type& type_to_construct,
                                        const feature_set& props,
                                        const std::vector<boost::intrusive_ptr<build_node> >& sources,
                                        const basic_build_target* t,
                                        const std::string* name,
                                        const main_target& owner) const
{
   feature_set* new_props = 0;
   typedef std::vector<boost::intrusive_ptr<build_node> > build_sources_t;
   build_sources_t modified_sources(sources);
   remove_dups(modified_sources);
   for(build_sources_t::const_iterator i = modified_sources.begin(); i != modified_sources.end(); ++i)
   {
      if ((**i).targeting_type_->equal_or_derived_from(searched_lib_))
      {
         // searched_lib may have <search> feature - add it to current build request
         const basic_build_target& lib_target = *(**i).products_.front();

         feature_set::const_iterator search_location = lib_target.properties().find("search");
         if (search_location != lib_target.properties().end())
         {
            if (!new_props)
               new_props = props.clone();

            new_props->join(*search_location);
         }
      }
   }

   return generator::construct(type_to_construct, new_props ? *new_props : props, modified_sources, t, name, owner);
}

}
