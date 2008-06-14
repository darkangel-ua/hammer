#pragma once

#include "basic_target.h"

namespace hammer
{
   class file_target : public basic_target
   {
      public:
         file_target(const main_target* mt, const pstring& name,
                     const hammer::type* t, const feature_set* f) : basic_target(mt, name, t, f) 
         {
         }

         virtual std::vector<boost::intrusive_ptr<build_node> > generate();
   };
}