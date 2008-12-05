#include "stdafx.h"
#include "main_target.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "generator_registry.h"
#include "build_node.h"

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
   return result;
}

const pstring& main_target::intermediate_dir() const
{
   return meta_target()->project()->intermediate_dir();
}

const location_t& main_target::location() const
{
   return meta_target()->project()->location();
}

void main_target::timestamp_info_impl(timestamp_info_t::getter_policy_t how_to_get) const
{
   throw std::logic_error("[main_target][FIXME] - this is bad target hierarchy");
}

}
