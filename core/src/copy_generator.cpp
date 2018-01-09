#include "stdafx.h"
#include <set>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem/convenience.hpp>
#include <hammer/core/copy_generator.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include <hammer/core/copy_main_target.h>
#include <hammer/core/generated_build_target.h>
#include <hammer/core/types.h>
#include <hammer/core/build_action.h>
#include <hammer/core/target_type.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/product_argument_writer.h>
#include <hammer/core/collect_nodes.h>

using namespace boost::assign;

namespace hammer{
namespace{
   
   class copy_action : public build_action
   {
      public:
         copy_action(const target_type& tag_type) 
            : build_action("copy file"),
              tag_writer_("", tag_type)
         {}

         std::string target_tag(const build_node& node,
                                const build_environment& environment) const override;

      protected:
         bool execute_impl(const build_node& node,
                           const build_environment& environment) const override;
         void clean_on_fail(const build_node& node,
                            const build_environment& environment) const override;
      
      private:
         product_argument_writer tag_writer_;
   };

   std::string copy_action::target_tag(const build_node& node, const build_environment& environment) const
   {
      std::ostringstream s;
      tag_writer_.write(s, node, environment);
      return s.str();
   }

   bool copy_action::execute_impl(const build_node& node, const build_environment& environment) const
   {
      assert(node.products_.size() == 1);
      assert(node.sources_.size() == 1);

      location_t destination = node.products_.front()->location() / 
                               node.products_.front()->name();
      location_t source = node.sources_.front().source_target_->location() / 
                          node.sources_.front().source_target_->name();
      destination.normalize();
      source.normalize();
      
      if (exists(destination))
         environment.remove(destination);

      environment.copy(source, destination);

      return true;
   }

   void copy_action::clean_on_fail(const build_node& node, const build_environment& environment) const
   {

   }
}

copy_generator::copy_generator(hammer::engine& e)
   : generator(e, "copy", consumable_types_t(),
                          list_of<produced_type>(e.get_type_registry().get(types::COPIED), 0),
                          true)
{
   std::unique_ptr<build_action> a(new copy_action(e.get_type_registry().get(types::COPIED)));
   action(std::move(a));
}

typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;
typedef std::set<const build_node*> visited_nodes_t;

build_nodes_t
copy_generator::construct(const target_type& type_to_construct, 
                          const feature_set& props,
                          const nodes_t& sources,
                          const basic_build_target* t,
                          const std::string* composite_target_name,
                          const main_target& owner) const
{
   assert(dynamic_cast<const copy_main_target*>(&owner));
   assert(composite_target_name);

   const copy_main_target& true_owner = static_cast<const copy_main_target&>(owner);
   
   nodes_t result;
   build_node::sources_t collected_nodes;
   visited_nodes_t visited_nodes;
   collect_nodes(collected_nodes, visited_nodes, sources, 
                 true_owner.types_to_copy(), true_owner.recursive());

   result.reserve(collected_nodes.size());
   for(build_node::sources_t::const_iterator i = collected_nodes.begin(), last = collected_nodes.end(); i != last; ++i)
   {
      boost::intrusive_ptr<build_node> new_node(new build_node(owner, false));
      new_node->targeting_type_ = &type_to_construct;
      new_node->action(action());
      new_node->sources_.push_back(*i);
      new_node->down_.push_back(i->source_node_);

      basic_build_target* new_target = new generated_2_build_target(&owner, i->source_target_->name(), true_owner.destination(), &type_to_construct, &props);
      new_node->products_.push_back(new_target);

      result.push_back(new_node);
   }

   return result;
}

}
