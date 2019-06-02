#include <assert.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/virtual_build_target.h>
#include <hammer/core/testing_compile_link_base_generator.h>

namespace hammer {

testing_compile_link_base_generator::testing_compile_link_base_generator(engine& e,
                                                                         std::string name,
                                                                         std::unique_ptr<generator> wrapped_generator,
                                                                         const type_tag& main_product_type,
                                                                         const bool failing_generator)
   : generator(e,
               name + "(" + wrapped_generator->name() + ")",
               wrapped_generator->consumable_types(),
               make_product_types(e, {main_product_type, types::TESTING_PASSED}),
               true,
               build_action_ptr{},
               wrapped_generator->constraints()),
     wrapped_generator_(move(wrapped_generator)),
     failing_generator_(failing_generator)
{
}

build_nodes_t
testing_compile_link_base_generator::construct(const target_type& type_to_construct,
                                               const feature_set& props,
                                               const build_nodes_t& sources,
                                               const basic_build_target* source_target,
                                               const std::string* composite_target_name,
                                               const main_target& owner) const
{
   build_nodes_t wrapped_nodes = wrapped_generator_->construct(*wrapped_generator_->producable_types()[0].type_, props, sources, source_target, composite_target_name, owner);
   // assume compile can produce only one node - can't imagine situations with many, right now
   assert(wrapped_nodes.size() == 1);

   const build_nodes_t this_nodes = generator::construct(type_to_construct, props, sources, source_target, composite_target_name, owner);
   // we always produce only one node
   assert(this_nodes.size() == 1);

   // we can't just replace compile_nodes.products on this_node.products because compile generator will not find expected product (obj for example)
   // so dispite the fact that we never actually produce wrapped producs in case of failing generator we need to include it into products, but
   // we add them as virtual targets
   wrapped_nodes.front()->products_.insert(wrapped_nodes.front()->products_.end(),
                                           this_nodes.front()->products_.begin(),
                                           this_nodes.front()->products_.end());
   return wrapped_nodes;
}

basic_build_target*
testing_compile_link_base_generator::create_target(const main_target* mt,
                                                   const build_node::sources_t& sources,
                                                   const std::string* target_name,
                                                   const produced_type& type,
                                                   const feature_set* f) const
{
   for (const auto& p : producable_types())
      if (*p.type_ == *type.type_)
         return generator::create_target(mt, sources, target_name, type, f);

   // for failing generators we create virtual targets, because wrapped generator will never create anything except output
   if (failing_generator_)
      return new virtual_build_target{mt, *target_name, type.type_, f, true};
   else
      return wrapped_generator_->create_target(mt, sources, target_name, type, f);
}

}
