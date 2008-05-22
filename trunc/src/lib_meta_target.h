#pragma once
#include "meta_target.h"

namespace hammer
{
   class lib_meta_target : public meta_target
   {
      public:
         lib_meta_target(hammer::project* p, const pstring& name, feature_set* fs,
                         feature_set* usage_req);
      
      protected:
         virtual const type* instantiate_type(const feature_set& fs) const;
   };
}