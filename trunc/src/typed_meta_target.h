#pragma once

#include "meta_target.h"

namespace hammer
{
   class type;

   class typed_meta_target : public meta_target
   {
      public:
         typed_meta_target(hammer::project* p, const pstring& name, 
                           const requirements_decl& req, 
                           const requirements_decl& usage_req,
                           const hammer::type& t) :  
            meta_target(p, name, req, usage_req), type_(&t)
         {
         }
         
         const hammer::type& type() const { return *type_; }

      protected:
         virtual main_target* construct_main_target(const feature_set* properties) const;

     private:
        const hammer::type* type_;
   };
}