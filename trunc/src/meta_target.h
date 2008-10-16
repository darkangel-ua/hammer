#if !defined(h_f089a0da_32ba_4ee0_ad36_f3e366a47018)
#define h_f089a0da_32ba_4ee0_ad36_f3e366a47018

#include "basic_meta_target.h"

namespace hammer
{
   class basic_target;
   class feature_set;
   class type;
   class generated_target;

   class meta_target : public basic_meta_target
   {
      public:
         meta_target(hammer::project* p, const pstring& name, 
                     const requirements_decl& props,
                     const requirements_decl& usage_req);
         virtual void instantiate(const main_target* owner, 
                                  const feature_set& build_request, 
                                  std::vector<basic_target*>* result, 
                                  feature_set* usage_requirements) const;

         virtual ~meta_target(){}

      protected:
         virtual main_target* construct_main_target(const feature_set* properties) const = 0;
         // compute usage requirements from target usage requirements and 
         // usage requirements that was calculated during instantiation meta targets from sources
         virtual void compute_usage_requirements(feature_set& result, 
                                                 const feature_set& full_build_request,
                                                 const feature_set& computed_usage_requirements) const;
      private:
         void instantiate_meta_targets(sources_decl& simple_targets,
                                       std::vector<basic_target*>& instantiated_meta_targets,
                                       feature_set& usage_requirements,
                                       const meta_targets_t& meta_targets,
                                       const feature_set& build_request,
                                       const main_target& owner_for_new_targets) const;

         // instantiate targets that we found in use feature and 
         // add additional usage requirements to main target usage requirements
         void compute_additional_usage_requirements(sources_decl& simple_targets,
                                                    std::vector<basic_target*>& instantiated_meta_targets,
                                                    feature_set& usage_requirements,
                                                    const sources_decl& sources_from_usage,
                                                    const feature_set& build_request,
                                                    const main_target& owner_for_new_targets) const;
   };
}

#endif //h_f089a0da_32ba_4ee0_ad36_f3e366a47018
