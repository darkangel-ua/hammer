#pragma once

#include "meta_target.h"

namespace hammer
{
   class pch_meta_target : public meta_target
   {
      public:
         pch_meta_target(hammer::project* p, const pstring& name, 
                         const requirements_decl& req, 
                         const requirements_decl& usage_req);
      protected:
         virtual main_target* construct_main_target(const feature_set* properties) const;
         virtual void compute_usage_requirements(feature_set& result, 
                                                 const feature_set& full_build_request,
                                                 const feature_set& computed_usage_requirements) const;
      
      private:
         mutable main_target* last_constructed_main_target_;
   };
}