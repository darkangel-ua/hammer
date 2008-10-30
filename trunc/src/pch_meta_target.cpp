#include "stdafx.h"
#include "pch_meta_target.h"
#include "types.h"
#include "project.h"
#include "engine.h"
#include "pch_main_target.h"
#include "feature_registry.h"
#include "feature.h"
#include "feature_set.h"

namespace hammer
{

pch_meta_target::pch_meta_target(hammer::project* p, const pstring& name, 
                                 const requirements_decl& req, 
                                 const requirements_decl& usage_req)
  : meta_target(p, name, req, usage_req), 
    last_constructed_main_target_(NULL)
{
}

main_target* pch_meta_target::construct_main_target(const feature_set* properties) const
{
  last_constructed_main_target_ = new(project()->engine()->targets_pool()) 
                                     pch_main_target(this, 
                                                     name(), 
                                                     &project()->engine()->get_type_registry().resolve_from_name(types::PCH), 
                                                     properties,
                                                     project()->engine()->targets_pool());

   return last_constructed_main_target_;
}

void pch_meta_target::compute_usage_requirements(feature_set& result, 
                                                 const feature_set& full_build_request,
                                                 const feature_set& computed_usage_requirements) const
{
   // adding __pch_target feature to usage requirements to mark dependent targets as built with pch
   this->usage_requirements().eval(full_build_request, &result);
   feature* pch_feature = project()->engine()->feature_registry().create_feature("__use_pch", "on");
   pch_feature->get_generated_data().target_ = last_constructed_main_target_;
   result.join(pch_feature);
}

sources_decl pch_meta_target::compute_additional_sources(const main_target& owner) const
{
   sources_decl result;
   const type_registry& tr = this->project()->engine()->get_type_registry();
   for(sources_decl::const_iterator i = owner.meta_target()->sources().begin(), last =  owner.meta_target()->sources().end(); i!= last; ++i)
      if (i->type(tr) == NULL) // that meta target
         result.push_back(*i);

   return result;
}

}