#include "stdafx.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "source_target.h"
#include "main_target.h"
#include "feature_set.h"
#include "feature_registry.h"
#include "feature.h"
#include "location.h"
#include "requirements_decl.h"

using namespace std;

namespace hammer{
   meta_target::meta_target(hammer::project* p, 
                            const pstring& name, 
                            const requirements_decl& props, 
                            const requirements_decl& usage_req) 
                           : basic_meta_target(p, name, props, usage_req)
   {
      requirements().insert_infront(p->requirements());
      usage_requirements().insert_infront(p->usage_requirements());
   }

   void meta_target::instantiate(const main_target* owner,
                                 const feature_set& build_request,
                                 std::vector<basic_target*>* result,
                                 feature_set* usage_requirements) const
   {
      feature_set* mt_fs = build_request.clone();
      requirements().eval(mt_fs, project()->engine()->feature_registry());
      project()->engine()->feature_registry().add_defaults(mt_fs);

      vector<basic_target*> sources;

      sources_t simple_targets;
      meta_targets_t meta_targets;
      split_sources(&simple_targets, &meta_targets);

      feature_set* build_request_with_propagated = build_request.clone();
      build_request_with_propagated->copy_propagated(*mt_fs);
      
      main_target* mt = new(project()->engine()->targets_pool()) 
                           main_target(this, 
                                       name(), 
                                       instantiate_type(*mt_fs), 
                                       mt_fs,
                                       project()->engine()->targets_pool());
      instantiate_meta_targets(meta_targets, *build_request_with_propagated, mt, &sources, usage_requirements);
      mt_fs->join(*usage_requirements);
      mt->properties(mt_fs);
      instantiate_simple_targets(simple_targets, *mt_fs, *mt, &sources);
      mt->sources(sources);
      this->usage_requirements().eval(*mt_fs, usage_requirements);
      
      result->push_back(mt);
   }
}