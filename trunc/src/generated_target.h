#pragma once
#include "basic_target.h"

namespace hammer
{
   class generated_target : public basic_target
   {
      public:
         generated_target(const main_target* mt, const pstring& n, 
                          const hammer::type* t, const feature_set* f);
         virtual const pstring& location() const;
         virtual std::vector<boost::intrusive_ptr<build_node> > generate();
   };
}