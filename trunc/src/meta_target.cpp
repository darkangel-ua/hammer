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

using namespace std;

namespace hammer{
   meta_target::meta_target(hammer::project* p, const pstring& name, 
                            const feature_set* fs) 
                           : project_(p), name_(name), features_(fs)
   {

   }

   void meta_target::insert(const pstring& source)
   {
      sources_.push_back(source);
   }

   void meta_target::insert(const std::vector<pstring>& srcs)
   {
      sources_.insert(sources_.end(), srcs.begin(), srcs.end());
   }

   std::vector<basic_target*> 
   meta_target::instantiate_source(main_target* owner, 
                                   const pstring& s, 
                                   const feature_set& build_request) const
   {
      feature_set* new_build_request = build_request.clone();
      new_build_request->add_propagated(owner->properties());
      std::vector<basic_target*> result;
      if (const meta_target* t = project_->find_target(s))
      {
         vector<basic_target*> r(t->instantiate(*new_build_request));
         result.insert(result.end(), r.begin(), r.end());
      }
      else
      {
         const type* tp = project_->engine()->get_type_registry().resolve_from_target_name(s, owner->properties());
         source_target* st = new(project_->engine()) source_target(owner, s, tp, &owner->properties());
         result.push_back(st);
      }

      return result;
   }
   
   std::vector<basic_target*> meta_target::instantiate(const feature_set& build_request) const
   {
      feature_set* mt_fs = features_->join(build_request);
      project_->engine()->feature_registry().add_defaults(mt_fs);
      main_target* mt = new(project_->engine()->targets_pool()) 
                           main_target(this, 
                                       name(), 
                                       instantiate_type(*mt_fs), 
                                       mt_fs,
                                       project_->engine()->targets_pool());

      vector<basic_target*> sources;
      for(sources_t::const_iterator i = sources_.begin(), last = sources_.end(); i != last; ++i)
      {
         vector<basic_target*> r = instantiate_source(mt, *i, build_request);
         sources.insert(sources.end(), r.begin(), r.end());
      }

      mt->sources(sources);
      
      vector<basic_target*> result;
      result.push_back(mt);
      
      return result;
   }
}