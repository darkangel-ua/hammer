#include "stdafx.h"
#include "testing_meta_target.h"
#include "project.h"
#include "engine.h"
#include "main_target.h"
#include "feature_set.h"
#include "feature.h"

namespace hammer{

testing_meta_target::testing_meta_target(hammer::project* p, 
                                         const pstring& name, 
                                         const requirements_decl& req, 
                                         const requirements_decl& usage_req,
                                         const hammer::type& t)
   : typed_meta_target(p, name, req, usage_req, t)
{

}

sources_decl testing_meta_target::compute_additional_sources(const main_target& owner) const
{
   sources_decl result;
   for(feature_set::const_iterator i = owner.properties().find("testing.additional-source"), last = owner.properties().end(); i != last;)
   {
      result.push_back((**i).get_dependency_data().source_);
      i = owner.properties().find(i + 1, "testing.additional-source");
   }

   return result;
}

}
