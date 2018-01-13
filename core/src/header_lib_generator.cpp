#include <hammer/core/header_lib_generator.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/virtual_build_target.h>
#include <hammer/core/generator_registry.h>

namespace hammer{

header_lib_generator::header_lib_generator(hammer::engine& e,
                                           const std::string& name,
                                           const consumable_types_t& source_types,
                                           const producable_types_t& target_types,
                                           const feature_set* c)
   :
    generator(e, name, source_types, 
              target_types, true, build_action_ptr{}, c),
    header_type_(e.get_type_registry().get(types::H))
{

}

build_nodes_t
header_lib_generator::construct(const target_type& type_to_construct,
                                const feature_set& props,
                                const std::vector<boost::intrusive_ptr<build_node> >& sources,
                                const basic_build_target* t,
                                const std::string* composite_target_name,
                                const main_target& owner) const
{
   typedef std::vector<boost::intrusive_ptr<build_node> > build_sources_t;
   build_sources_t result;

   // add HEADER_LIB node to result
   boost::intrusive_ptr<build_node> header_lib_node(new build_node(owner, true, build_action_ptr{}));
   header_lib_node->targeting_type_ = &type_to_construct;
   result.push_back(header_lib_node);
   std::auto_ptr<virtual_build_target> header_lib_product(new virtual_build_target(&owner, *composite_target_name, &type_to_construct, &props, true));
   header_lib_node->products_.push_back(header_lib_product.get());
   header_lib_product.release();

   return result;
}

void add_header_lib_generator(engine& e, generator_registry& gr)
{
   generator::consumable_types_t source;
   generator::producable_types_t target;
   source.push_back(generator::consumable_type(e.get_type_registry().get(types::H), 0, 0));
   target.push_back(generator::produced_type(e.get_type_registry().get(types::HEADER_LIB), 1));
   std::unique_ptr<generator> g(new header_lib_generator(e, "header_lib.linker", source, target));
   gr.insert(std::move(g));
}

}
