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

using namespace std;

namespace hammer{

main_target::main_target(const hammer::meta_target* mt, 
                         const pstring& name, 
                         const hammer::type* t, 
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

void main_target::generate_and_add_dependencies(hammer::build_node& node)
{
   build_nodes_t result;
   for(dependencies_t::iterator i = dependencies_.begin(), last = dependencies_.end(); i != last; ++i)
   {
      build_nodes_t tmp(static_cast<main_target*>(*i)->generate());
      result.insert(result.end(), tmp.begin(), tmp.end());
   }

   node.dependencies_.insert(node.dependencies_.end(), result.begin(), result.end());
}

std::vector<boost::intrusive_ptr<build_node> > 
main_target::generate()
{
   if (generate_cache_filled_)
      return generate_cache_;
   else
   {
      std::vector<boost::intrusive_ptr<hammer::build_node> >  result(meta_target_->project()->engine()->generators().construct(this));
      build_node_ = result.front();
      generate_and_add_dependencies(*build_node_);
      add_additional_dependencies(*build_node_);
      generate_cache_ = result;
      generate_cache_filled_ = true;
      return result;
   }
}

void main_target::add_additional_dependencies(hammer::build_node& generated_node) const
{
   boost::intrusive_ptr<hammer::build_node> int_dir_node(new hammer::build_node);
   int_dir_node->products_.push_back(new directory_target(this, intermediate_dir()));
   int_dir_node->action(static_cast<const directory_target*>(int_dir_node->products_.front())->action());

   generated_node.dependencies_.push_back(int_dir_node);
}

const location_t& main_target::intermediate_dir() const
{
   if (intermediate_dir_.empty())
   {
      ostringstream s;
      dump_for_hash(s, properties());
      boost::crypto::md5 md5(s.str());
      
      intermediate_dir_ = meta_target()->project()->location() / ".hammer/bin" / name().to_string() / md5.to_string();
   }

   return intermediate_dir_;
}

const location_t& main_target::location() const
{
   return meta_target()->project()->location();
}

void main_target::timestamp_info_impl() const
{
   throw std::logic_error("[main_target][FIXME] - this is bad target hierarchy");
}

}
