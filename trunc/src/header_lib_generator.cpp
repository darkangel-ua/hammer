#include "stdafx.h"
#include "header_lib_generator.h"
#include "types.h"
#include "engine.h"
#include "type_registry.h"

namespace hammer{

header_lib_generator::header_lib_generator(hammer::engine& e,
                                           const std::string& name,
                                           const consumable_types_t& source_types,
                                           const producable_types_t& target_types,
                                           const feature_set* c)
   :
    generator(e, name, source_types, 
              target_types, true, c),
    header_type_(e.get_type_registry().resolve_from_name(types::H))
{

}

std::vector<boost::intrusive_ptr<build_node> >
header_lib_generator::construct(const type& target_type, 
                                const feature_set& props,
                                const std::vector<boost::intrusive_ptr<build_node> >& sources,
                                const basic_target* t,
                                const pstring* composite_target_name,
                                const main_target& owner) const
{
   typedef std::vector<boost::intrusive_ptr<build_node> > build_sources_t;
   build_sources_t modified_sources(sources);
   build_sources_t extracted_products;
   for(build_sources_t::iterator i = modified_sources.begin(); i != modified_sources.end();)
   {
      if (*(**i).targeting_type_ != header_type_)
      {
         extracted_products.push_back(*i);
         
         i = modified_sources.erase(i);
      }
      else
         ++i;
   }

   std::vector<boost::intrusive_ptr<build_node> > result(generator::construct(target_type, props, modified_sources, t, composite_target_name, owner));
   result.insert(result.end(), extracted_products.begin(), extracted_products.end());
   return result;
}

}