#include "stdafx.h"
#include <boost/unordered_map.hpp>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/build_node.h>
#include <hammer/core/directory_build_target.h>
#include <boost/crypto/md5.hpp>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/output_location_strategy.h>
#include <hammer/core/types.h>
#include "mksig_action.h"
#include "signature_build_target.h"
#include <hammer/core/hashed_location.h>
#include <set>

using namespace std;

namespace hammer{

static
std::shared_ptr<mksig_action> mksig_action_ = std::make_shared<mksig_action>();

main_target::main_target(const basic_meta_target* mt,
                         const std::string& name,
                         const hammer::target_type* t,
                         const feature_set* props)
   : basic_target(this, name, t, props), meta_target_(mt)
{
}

void main_target::sources(const std::vector<basic_target*>& srcs)
{
   sources_ = srcs;
}

void main_target::dependencies(const dependencies_t& deps)
{
   dependencies_ = deps;
}

void main_target::generate_and_add_dependencies(build_nodes_t& nodes) const
{
   build_nodes_t dependency_nodes;
   for (dependencies_t::const_iterator i = dependencies_.begin(), last = dependencies_.end(); i != last; ++i) {
      build_nodes_t tmp((*i)->generate());
      dependency_nodes.insert(dependency_nodes.end(), tmp.begin(), tmp.end());
   }

   add_this_target_dependency(nodes, dependency_nodes);
}

build_nodes_t
main_target::generate_impl() const
{
   build_nodes_t result = get_engine().generators().construct(this);

   build_nodes_t owned_nodes;
   for (auto& node : result)
      if (&node->products_owner() == this)
         owned_nodes.push_back(node);

   if (!result.empty())
      build_node_ = result.front();

   generate_and_add_dependencies(owned_nodes);
   add_additional_dependencies(owned_nodes);

   return result;
}

static
void collect_locations(const main_target* this_,
                       boost::unordered_map<hashed_location, build_node*>& locations,
                       build_node& node)
{
   for (const basic_build_target* bt : node.products_) {
      // virtual targets have empty location - skip them
      if (!bt->location().empty())
         locations.insert({bt->location(), &node});
   }

   for (build_node::source_t& s : node.sources_) {
      // inspect only intermediate source nodes
      if (&s.source_node_->products_owner() == this_ && !s.source_node_->sources_.empty())
         collect_locations(this_, locations, *s.source_node_);
   }
}

build_nodes_t
main_target::create_intermediate_dirs_build_nodes(const build_nodes_t& build) const
{
   boost::unordered_map<hashed_location, hammer::build_node*> locations;
   for (auto& node : build) {
      if (&node->products_owner() != this)
         continue;

      collect_locations(this, locations, *node);
   }

   build_nodes_t result;
   for (const auto& l : locations) {
      directory_build_target* intermediate_dir_target = new directory_build_target(this, l.first.location());
      build_node_ptr intermediate_dir_node(new hammer::build_node(*this, false, intermediate_dir_target->action()));
      intermediate_dir_node->products_.push_back(intermediate_dir_target);
      result.push_back(intermediate_dir_node);

      if (need_signature()) {
         signature_build_target* signature_target =
            l.second->is_composite() ? new signature_build_target(this, *l.second)
                                     : new signature_build_target(this, l.second->build_request(), l.first.location());

         build_node_ptr signature_node { new hammer::build_node(*this, false, mksig_action_) };
         signature_node->products_.push_back(signature_target);
         signature_node->dependencies_.push_back(intermediate_dir_node);
         l.second->dependencies_.push_back(signature_node);
      }
   }

   return result;
}

void main_target::add_additional_dependencies(build_nodes_t& generated_nodes) const
{
   add_this_target_dependency(generated_nodes, create_intermediate_dirs_build_nodes(generated_nodes));
}

// search for all leaf nodes and add hamfile_node to it as dependency
void main_target::add_this_target_dependency(build_nodes_t& nodes,
                                             const build_nodes_t& dependencies) const
{
   for (auto& node : nodes)
      add_this_target_dependency(*node, dependencies);
}

void main_target::add_this_target_dependency(hammer::build_node& node,
                                             const build_nodes_t& dependencies) const
{
   for (hammer::build_node::sources_t::iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i) {
      if (&i->source_node_->products_owner() == this) {
         if (i->source_node_->sources_.empty())
            node.dependencies_.insert(node.dependencies_.begin(), dependencies.begin(), dependencies.end());
         else
            add_this_target_dependency(*i->source_node_, dependencies);
      }
   }
}

location_t main_target::intermediate_dir_impl() const
{
   return get_engine().output_location_strategy().compute_output_location(*this);
}

const location_t& main_target::intermediate_dir() const
{
   if (intermediate_dir_.empty())
      intermediate_dir_ = intermediate_dir_impl();

   return intermediate_dir_;
}

const location_t& main_target::location() const
{
   return meta_target_->location();
}

std::string main_target::version() const
{
   feature_set::const_iterator i = properties().find("version");
   if (i != properties().end())
      return (**i).value();
   else
      return string();
}

void main_target::additional_hash_string_data(std::ostream& s) const
{
   typedef set<const main_target*> main_target_sources_t;

   main_target_sources_t main_target_sources;
   for(sources_t::const_iterator i = sources_.begin(), last = sources_.end(); i != last; ++i)
      if (this != (**i).get_main_target())
         main_target_sources.insert((**i).get_main_target());

   vector<string> hashes;
   for(main_target_sources_t::const_iterator i = main_target_sources.begin(), last = main_target_sources.end(); i != last; ++i)
      hashes.push_back((**i).name() + '-' + (**i).hash_string());

   sort(hashes.begin(), hashes.end());

   bool first = true;
   for(vector<string>::const_iterator i = hashes.begin(), last = hashes.end(); i != last; ++i)
   {
      if (!first)
         s << ' ';
      else
         first = false;

      s << *i;
   }
}

}
