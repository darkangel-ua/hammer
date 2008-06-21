#include "stdafx.h"
#include "exe_and_shared_lib_generator.h"
#include "engine.h"
#include "type_registry.h"
#include "types.h"
#include "feature_set.h"
#include "feature.h"
#include "basic_target.h"

namespace hammer{

exe_and_shared_lib_generator::exe_and_shared_lib_generator(hammer::engine& e,
                                                           const std::string& name,
                                                           const consumable_types_t& source_types,
                                                           const producable_types_t& target_types,
                                                           bool composite,
                                                           const feature_set* c)
                                                          : generator(e, name, source_types, 
                                                                      target_types, composite, c)

{
   searched_lib_ = &e.get_type_registry().resolve_from_name(types::SEARCHED_LIB);
}

std::vector<boost::intrusive_ptr<build_node> >
exe_and_shared_lib_generator::construct(const type& target_type, 
                                        const feature_set& props,
                                        const std::vector<boost::intrusive_ptr<build_node> >& sources,
                                        const basic_target* t,
                                        const pstring* name) const
{
   feature_set* new_props = 0;
   typedef std::vector<boost::intrusive_ptr<build_node> > build_sources_t;
   build_sources_t modified_sources(sources);
   for(build_sources_t::iterator i = modified_sources.begin(); i != modified_sources.end();)
   {
      if (*(**i).targeting_type_ == *searched_lib_)
      {
         if (!new_props)
            new_props = props.clone();
         
         // searched_lib produce only one products - self.
         const basic_target& lib_target = *(**i).products_.front();

         const feature& lib_name = *lib_target.properties().get("name");
         new_props->join("__searched_lib_name", lib_name.value().begin());

         feature_set::const_iterator search_location = lib_target.properties().find("search");
         if (search_location != lib_target.properties().end())
            new_props->join(*search_location);

         i = modified_sources.erase(i);
      }
      else
         ++i;
   }

   return generator::construct(target_type, new_props ? *new_props : props, modified_sources, t, name);
}

}