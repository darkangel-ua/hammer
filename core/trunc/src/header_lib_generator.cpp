#include "stdafx.h"
#include <hammer/core/header_lib_generator.h>
#include <hammer/core/types.h>
#include <hammer/core/type.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/header_lib_target.h>

namespace hammer{

header_lib_generator::header_lib_generator(hammer::engine& e,
                                           const std::string& name,
                                           const consumable_types_t& source_types,
                                           const producable_types_t& target_types,
                                           const feature_set* c)
   :
    generator(e, name, source_types, 
              target_types, true, c),
    header_type_(e.get_type_registry().get(types::H))
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
   build_sources_t result;

   // add HEADER_LIB node to result
   boost::intrusive_ptr<build_node> header_lib_node(new build_node);
   header_lib_node->targeting_type_ = &target_type;
   result.push_back(header_lib_node);
   std::auto_ptr<header_lib_target> header_lib_product(new header_lib_target(&owner, *composite_target_name, &target_type, &props));
   header_lib_node->products_.push_back(header_lib_product.get());
   header_lib_product.release();

   // filter out H targets
   for(build_sources_t::const_iterator i = sources.begin(); i != sources.end(); ++i)
      if (!(**i).targeting_type_->equal_or_derived_from(header_type_))
         result.push_back(*i);
   
   return result;
}

}