#include "stdafx.h"
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/build_node.h>
#include <hammer/core/directory_target.h>
#include <boost/crypto/md5.hpp>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/output_location_strategy.h>
#include <hammer/core/generated_target.h>
#include <hammer/core/types.h>
#include "mksig_action.h"
#include "signature_target.h"
#include <set>

using namespace std;

namespace hammer{

boost::shared_ptr<mksig_action> main_target::mksig_action_ = boost::shared_ptr<mksig_action>(new mksig_action);

main_target::main_target(const hammer::meta_target* mt,
                         const pstring& name,
                         const hammer::target_type* t,
                         const feature_set* props,
                         pool& p)
   : basic_target(this, name, t, props), meta_target_(mt),
     generate_cache_filled_(false)
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

void main_target::generate_and_add_dependencies(hammer::build_node& node) const
{
   build_nodes_t dependency_nodes;
   for(dependencies_t::const_iterator i = dependencies_.begin(), last = dependencies_.end(); i != last; ++i)
   {
      build_nodes_t tmp((*i)->generate());
      dependency_nodes.insert(dependency_nodes.end(), tmp.begin(), tmp.end());
   }

   add_this_target_dependency(node, dependency_nodes);
}

std::vector<boost::intrusive_ptr<build_node> >
main_target::generate() const
{
   if (generate_cache_filled_)
      return generate_cache_;
   else
   {
      std::vector<boost::intrusive_ptr<hammer::build_node> >  result(get_engine()->generators().construct(this));
      build_node_ = result.front();
      generate_and_add_dependencies(*build_node_);
      add_additional_dependencies(*build_node_);
      generate_cache_ = result;
      generate_cache_filled_ = true;
      return result;
   }
}

build_node_ptr 
main_target::create_intermediate_dir_dependency() const
{
   build_node_ptr int_dir_node(new hammer::build_node(*this, false));
   int_dir_node->products_.push_back(new directory_target(this, intermediate_dir()));
   int_dir_node->action(static_cast<const directory_target*>(int_dir_node->products_.front())->action());
   
   return int_dir_node;
}

void main_target::add_additional_dependencies(hammer::build_node& generated_node) const
{
   build_node_ptr intr_dir_node = create_intermediate_dir_dependency();
   add_hamfile_dependency(generated_node, intr_dir_node);
}

void main_target::add_hamfile_dependency(hammer::build_node& node,
                                         const build_node_ptr& intermediate_dir_node) const
{
   boost::intrusive_ptr<hammer::build_node> signature_node(new hammer::build_node(*this, false));
   signature_node->products_.push_back(
      new signature_target(this, 
                           pstring(get_engine()->pstring_pool(), name().to_string() + ".target.sig"), 
                           &get_engine()->get_type_registry().get(types::UNKNOWN),
                           &properties()));
   
   signature_node->action(mksig_action_.get());
   signature_node->dependencies_.push_back(intermediate_dir_node);

   add_this_target_dependency(node, build_nodes_t(1, signature_node));
}

// search for all leaf nodes and add hamfile_node to it as dependency
bool main_target::add_this_target_dependency(hammer::build_node& node,
                                             const build_nodes_t& nodes) const
{
   for(hammer::build_node::sources_t::iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i) {
      if (&i->source_node_->products_owner() == this) {
         if (i->source_node_->sources_.empty())
            node.dependencies_.insert(node.dependencies_.begin(), nodes.begin(), nodes.end());
         else
            add_this_target_dependency(*i->source_node_, nodes);
      }
   }

   return false;
}

location_t main_target::intermediate_dir_impl() const
{
   return get_engine()->output_location_strategy().compute_output_location(*this);
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

void main_target::timestamp_info_impl() const
{
   throw std::logic_error("[main_target][FIXME] - this is bad target hierarchy");
}

std::string main_target::version() const
{
   feature_set::const_iterator i = properties().find("version");
   if (i != properties().end())
      return (**i).value().to_string();
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
      hashes.push_back((**i).name().to_string() + '-' + (**i).hash_string());

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
