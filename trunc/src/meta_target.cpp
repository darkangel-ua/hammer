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
   
   void meta_target::instantiate_meta_targets(const meta_targets_t& targets, 
                                              const feature_set& build_request,
                                              std::vector<basic_target*>* result, 
                                              feature_set* usage_requirments) const
   {
      for(meta_targets_t::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
      {
         vector<basic_target*> r((**i).instantiate(build_request));
         result->insert(result->end(), r.begin(), r.end());
      }
   }

   void meta_target::instantiate_simple_targets(const sources_t& targets, 
                                                const feature_set& build_request,
                                                const main_target& owner, 
                                                std::vector<basic_target*>* result) const
   {
      for(sources_t::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
      {
         const type* tp = project_->engine()->get_type_registry().resolve_from_target_name(*i, owner.properties());
         source_target* st = new(project_->engine()) source_target(&owner, *i, tp, &owner.properties());
         result->push_back(st);
      }
   }
   
   std::vector<basic_target*> meta_target::instantiate(const feature_set& build_request) const
   {
      feature_set* mt_fs = features_->join(build_request);
      project_->engine()->feature_registry().add_defaults(mt_fs);

      vector<basic_target*> sources;
      feature_set* usage_requirments = project()->engine()->feature_registry().make_set();

      sources_t simple_targets;
      meta_targets_t meta_targets;
      split_sources(&simple_targets, &meta_targets);

      instantiate_meta_targets(meta_targets, *mt_fs, &sources, usage_requirments);
      
      mt_fs->join(*usage_requirments);
      main_target* mt = new(project_->engine()->targets_pool()) 
                           main_target(this, 
                                       name(), 
                                       instantiate_type(*mt_fs), 
                                       mt_fs,
                                       project_->engine()->targets_pool());

      instantiate_simple_targets(simple_targets, *mt_fs, *mt, &sources);
      
      mt->sources(sources);
      
      vector<basic_target*> result;
      result.push_back(mt);
      
      return result;
   }

   void meta_target::split_sources(sources_t* simple_targets, meta_targets_t* meta_targets) const
   {
      for(sources_t::const_iterator i = sources_.begin(), last = sources_.end(); i != last; ++i)
      {
         if (const meta_target* t = project_->find_target(*i))
            meta_targets->push_back(t);
         else
            simple_targets->push_back(*i);
     }
   }

}