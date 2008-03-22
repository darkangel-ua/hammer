#pragma once

#include "meta_target.h"

namespace hammer
{
   class type;

   class typed_meta_target : public meta_target
   {
      public:
         typed_meta_target(hammer::project* p, const pstring& name, 
                           const feature_set* fs, const hammer::type& t) :  
            meta_target(p, name, fs), type_(&t)
         {
         }
         
         const hammer::type& type() const { return *type_; }

      protected:
         virtual const hammer::type* instantiate_type() const { return type_; }

     private:
        const hammer::type* type_;
   };
}