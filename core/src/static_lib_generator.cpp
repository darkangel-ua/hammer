#include "stdafx.h"
#include <hammer/core/static_lib_generator.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/basic_target.h>

namespace hammer{

static_lib_generator::static_lib_generator(hammer::engine& e,
                                           const std::string& name,
                                           const consumable_types_t& source_types,
                                           const producable_types_t& target_types,
                                           bool composite,
                                           const build_action_ptr& action,
                                           const feature_set* c)
   : generator(e, name, source_types, target_types, composite, action, c),
     shared_lib_(e.get_type_registry().get(types::SHARED_LIB)),
     static_lib_(e.get_type_registry().get(types::STATIC_LIB)),
     searched_lib_(e.get_type_registry().get(types::SEARCHED_LIB))
{
}

build_nodes_t
static_lib_generator::construct(const target_type& type_to_construct,
                                const feature_set& props,
                                const std::vector<boost::intrusive_ptr<build_node> >& sources,
                                const basic_build_target* t,
                                const std::string* name,
                                const main_target& owner) const
{
   typedef std::vector<boost::intrusive_ptr<build_node> > build_sources_t;
   build_sources_t modified_sources(sources);
   build_sources_t extracted_products;
   for(build_sources_t::iterator i = modified_sources.begin(); i != modified_sources.end();)
   {
      if ((**i).targeting_type_->equal_or_derived_from(shared_lib_) || 
          (**i).targeting_type_->equal_or_derived_from(static_lib_) ||
          (**i).targeting_type_->equal_or_derived_from(searched_lib_))
      {
         extracted_products.push_back(*i);
         
         i = modified_sources.erase(i);
      }
      else
         ++i;
   }

   build_nodes_t result(generator::construct(type_to_construct, props, modified_sources, t, name, owner));
   result.insert(result.end(), extracted_products.begin(), extracted_products.end());

   return result;
}

}
