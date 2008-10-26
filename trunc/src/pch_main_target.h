#pragma once

#include "main_target.h"

namespace hammer
{
   class pch_main_target : public main_target
   {
      public:
         pch_main_target(const hammer::meta_target* mt, 
                         const pstring& name, 
                         const hammer::type* t, 
                         const feature_set* props,
                         pool& p)
                        : 
                         main_target(mt, name, t, props, p)
         {}

         virtual std::vector<boost::intrusive_ptr<hammer::build_node> > generate();
   };
}