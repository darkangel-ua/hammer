#include "stdafx.h"
#include "copy_generator.h"
#include "engine.h"
#include "type_registry.h"
#include <boost/assign/list_of.hpp>
#include "types.h"
#include "copy_main_target.h"
#include "copy_target.h"
#include "generated_target.h"
#include <set>
#include "types.h"
#include "build_action.h"
#include "type.h"
#include "build_environment.h"
#include "fs_helpers.h"
#include "product_argument_writer.h"
#include "collect_nodes.h"

using namespace boost::assign;

namespace hammer{
namespace{
   
   class copy_action : public build_action
   {
      public:
         copy_action(const type& tag_type) 
            : build_action("copy file"),
              tag_writer_("", tag_type)
         {}

         virtual std::string target_tag(const build_node& node, const build_environment& environment) const;

      protected:
         virtual bool execute_impl(const build_node& node, const build_environment& environment) const;
         virtual void clean_on_fail(const build_node& node, const build_environment& environment) const;
      
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
      assert(dynamic_cast<const copy_target*>(node.products_.front()) != NULL);

      location_t destination = node.products_.front()->location() / 
                               node.products_.front()->name().to_string();
      location_t source = node.sources_.front().source_target_->location() / 
                          node.sources_.front().source_target_->name().to_string();
      destination.normalize();
      source.normalize();
      destination = relative_path(destination, environment.current_directory());
      source = relative_path(source, environment.current_directory()); 
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
   std::auto_ptr<build_action> a(new copy_action(e.get_type_registry().get(types::COPIED)));
   action(a);
}

typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;
typedef std::set<const build_node*> visited_nodes_t;

nodes_t copy_generator::construct(const type& target_type, 
                                  const feature_set& props,
                                  const nodes_t& sources,
                                  const basic_target* t,
                                  const pstring* composite_target_name,
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
      boost::intrusive_ptr<build_node> new_node(new build_node);
      new_node->targeting_type_ = &target_type;
      new_node->action(action());
      new_node->sources_.push_back(*i);

      copy_target* new_target = new copy_target(&owner, i->source_target_->name(), &target_type, &props);
      new_node->products_.push_back(new_target);

      result.push_back(new_node);
   }

   return result;
}

}
