#pragma once
#include "main_target.h"

namespace hammer
{
   class searched_lib_main_target : public main_target
   {
      public:
         searched_lib_main_target(const hammer::meta_target* mt, 
                                  const pstring& name, 
                                  const feature_set* props,
                                  pool& p);
         virtual std::vector<boost::intrusive_ptr<hammer::build_node> > generate();

   };
}