#include <hammer/core/header_lib_generator.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/virtual_build_target.h>
#include <hammer/core/generator_registry.h>

namespace hammer {

header_lib_generator::header_lib_generator(hammer::engine& e,
                                           const std::string& name,
                                           const consumable_types_t& source_types,
                                           const producable_types_t& target_types)
   :
    generator(e, name, source_types, 
              target_types, true, build_action_ptr{}),
    header_type_(e.get_type_registry().get(types::H)),
    shared_lib_(e.get_type_registry().get(types::SHARED_LIB)),
    static_lib_(e.get_type_registry().get(types::STATIC_LIB)),
    searched_lib_(e.get_type_registry().get(types::SEARCHED_LIB))
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
   // just lift libs up
   build_nodes_t result;
   for (const auto& s : sources) {
      if (s->targeting_type_->equal_or_derived_from(shared_lib_) ||
          s->targeting_type_->equal_or_derived_from(static_lib_) ||
          s->targeting_type_->equal_or_derived_from(searched_lib_))
      {
         result.push_back(s);
      }
   }

   return result;
}

void add_header_lib_generator(engine& e,
                              generator_registry& gr)
{
   const auto sources = make_consume_types(e, {types::H, types::SHARED_LIB, types::STATIC_LIB, types::SEARCHED_LIB});
   const auto products = make_product_types(e, {types::HEADER_LIB});
   std::unique_ptr<generator> g(new header_lib_generator(e, "header_lib", sources, products));
   gr.insert(std::move(g));
}

}
