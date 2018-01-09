#if !defined(h_f089a0da_32ba_4ee0_ad36_f3e366a47018)
#define h_f089a0da_32ba_4ee0_ad36_f3e366a47018

#include "basic_meta_target.h"

namespace hammer
{
   class basic_target;
   class feature_set;
   class target_type;
   class generated_target;

   class meta_target : public basic_meta_target
   {
      public:
         meta_target(hammer::project* p, 
                     const std::string& name,
                     const requirements_decl& props,
                     const requirements_decl& usage_req);

      protected:
			void instantiate_impl(const main_target* owner,
			                      const feature_set& build_request,
			                      std::vector<basic_target*>* result,
			                      feature_set* usage_requirements) const override;

         // Now used for PCH support. PCH meta target import meta target sources from owner to reduce typing for msvc pch support
         // pch stdafx : stdafx.cpp stdafx.h ;
         // test a : main.cpp stdafx /boost/regex ;
         // To build stdafx target we need specify /boost/regex in stdafx sources too. But this is stupid double typing.
         virtual sources_decl compute_additional_sources(const main_target& owner) const;
         virtual main_target* construct_main_target(const main_target* owner, const feature_set* properties) const = 0;
         // compute usage requirements from this main target's usage requirements and 
         // usage requirements that was calculated during instantiation source meta targets
         virtual void compute_usage_requirements(feature_set& result, 
                                                 const main_target& constructed_target,
                                                 const feature_set& build_request,
                                                 const feature_set& computed_usage_requirements,
                                                 const main_target* owner) const;
         
   };
}

#endif //h_f089a0da_32ba_4ee0_ad36_f3e366a47018
