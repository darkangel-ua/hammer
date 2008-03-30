#include "stdafx.h"
#include "main_target.h"

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

std::vector<basic_target*> main_target::generate()
{
   return std::vector<basic_target*>();
}

}
