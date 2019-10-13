#include <hammer/core/types.h>
#include <hammer/core/engine.h>
#include <hammer/core/target_type.h>
#include <hammer/core/basic_build_target.h>
#include <hammer/core/testing_main_target.h>

namespace hammer {

testing_main_target::testing_main_target(const basic_meta_target* mt,
                    const std::string& name,
                    const target_type* t,
                    const feature_set* props)
   : main_target(mt, name, t, props)
{}

build_nodes_t
testing_main_target::generate_impl() const {
   build_nodes_t nodes = main_target::generate_impl();
   // we expect that testing generator returns only one node
   assert(nodes.size() == 1);

   const build_node_ptr& testing_node = nodes.front();
   auto result = build_node_ptr{new build_node{*this, true, {}}};
   result->targeting_type_ = testing_node->targeting_type_;

   // we filtering out from products everything except output_type
   const target_type& output_type = get_engine().get_type_registry().get(types::TESTING_OUTPUT);
   for (const auto& product : testing_node->products_) {
      if (product->type().equal_or_derived_from(output_type))
         result->products_.push_back(product);

      result->sources_.emplace_back(product, testing_node);
   }
   result->down_.push_back(testing_node);

   return { std::move(result) };
}

}
