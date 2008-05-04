#include "stdafx.h"
#include "main_target.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "generator_registry.h"

namespace hammer{

main_target::main_target(const hammer::meta_target* mt, 
                         const pstring& name, 
                         const hammer::type* t, 
                         const feature_set* props,
                         pool& p)
                        : basic_target(this, name, t, props), mt_(mt)
{
}

void main_target::sources(const std::vector<basic_target*>& srcs)
{
   sources_ = srcs;
}

boost::intrusive_ptr<build_node> main_target::generate()
{
   return mt_->project()->engine()->generators().construct(this);
}

const pstring& main_target::intermediate_dir() const
{
   return meta_target()->project()->intermediate_dir();
}

// const pstring& main_target::location() const
// {
//    return meta_target()->project()->location();
// }

}
