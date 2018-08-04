#include <assert.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/target_type.h>
#include <hammer/core/virtual_build_target.h>
#include <hammer/core/testing_compile_link_base_generator.h>

namespace hammer {

testing_compile_link_base_generator::testing_compile_link_base_generator(engine& e,
                                                                         const std::string& name,
                                                                         std::unique_ptr<generator> compile_generator,
                                                                         const generator::producable_types_t& products,
                                                                         const type_tag& virtual_target_type)
   : generator(e,
               name + "(" + compile_generator->name() + ")",
               compile_generator->consumable_types(),
               products,
               true,
               build_action_ptr{},
               compile_generator->constraints()),
     compile_generator_(move(compile_generator)),
     virtual_target_type_(e.get_type_registry().get(virtual_target_type))
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
   build_nodes_t compile_nodes = compile_generator_->construct(*compile_generator_->producable_types()[0].type_, props, sources, source_target, composite_target_name, owner);
   // assume compile can produce only one node - can't imagine situations with many, right now
   assert(compile_nodes.size() == 1);

   const build_nodes_t this_nodes = generator::construct(type_to_construct, props, sources, source_target, composite_target_name, owner);
   // we always produce only one node
   assert(this_nodes.size() == 1);

   // we can't just replace compile_nodes.products on this_node.products because compile generator will not find expected product (obj for example)
   // so dispite the fact that we never produce compiled output we need to include it into products
   // this will also lead to build-always situation because we will have products that never built
   compile_nodes.front()->products_.insert(compile_nodes.front()->products_.end(),
                                           this_nodes.front()->products_.begin(),
                                           this_nodes.front()->products_.end());
   return compile_nodes;
}

basic_build_target*
testing_compile_link_base_generator::create_target(const main_target* mt,
                                                   const build_node::sources_t& sources,
                                                   const std::string* target_name,
                                                   const produced_type& type,
                                                   const feature_set* f) const
{
   if (virtual_target_type_ == *type.type_)
      return new virtual_build_target(mt, *target_name, type.type_, f, false);

   return generator::create_target(mt, sources, target_name, type, f);
}

}
