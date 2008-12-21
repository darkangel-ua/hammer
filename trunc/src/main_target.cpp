#include "stdafx.h"
#include "main_target.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "generator_registry.h"
#include "build_node.h"
#include "directory_target.h"
#include <boost/crypto/md5.hpp>
#include "feature_set.h"

using namespace std;

namespace hammer{

main_target::main_target(const hammer::meta_target* mt, 
                         const pstring& name, 
                         const hammer::type* t, 
                         const feature_set* props,
                         pool& p)
                        : basic_target(this, name, t, props), meta_target_(mt)
{
}

void main_target::sources(const std::vector<basic_target*>& srcs)
{
   sources_ = srcs;
}

std::vector<boost::intrusive_ptr<build_node> > 
main_target::generate()
{
   std::vector<boost::intrusive_ptr<hammer::build_node> >  result(meta_target_->project()->engine()->generators().construct(this));
   build_node_ = result.front();
   boost::intrusive_ptr<hammer::build_node> int_dir_node(new hammer::build_node);
   int_dir_node->products_.push_back(new directory_target(this, intermediate_dir()));
   int_dir_node->action(static_cast<const directory_target*>(int_dir_node->products_.front())->action());

   build_node_->dependencies_.push_back(int_dir_node);

   return result;
}

const location_t& main_target::intermediate_dir() const
{
   if (intermediate_dir_.empty())
   {
      ostringstream s;
      dump_for_hash(s, properties());
      boost::crypto::md5 md5(s.str());
      
      intermediate_dir_ = meta_target()->project()->location() / ".hammer/bin" / md5.to_string();
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
