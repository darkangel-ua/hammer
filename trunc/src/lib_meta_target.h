#pragma once
#include "meta_target.h"

namespace hammer
{
   class lib_meta_target : public meta_target
   {
      public:
         lib_meta_target(hammer::project* p, const pstring& name, const feature_set& fs);
      
      protected:
         virtual const type* instantiate_type() const;
      
      private:
         mutable const type* type_;
   };
}