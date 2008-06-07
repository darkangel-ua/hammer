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
#include <boost/regex.hpp>
#include "location.h"
#include "requirements_decl.h"

using namespace std;

namespace hammer{
   meta_target::meta_target(hammer::project* p, 
                            const pstring& name, 
                            const requirements_decl& props, 
                            const requirements_decl& usage_req) 
                           : basic_meta_target(name, props, usage_req), project_(p) 
   {
      requirements().insert_infront(p->requirements());
      usage_requirements().insert_infront(p->usage_requirements());
   }

   const location_t& meta_target::location() const
   {
      return project_->location();
   }

   void meta_target::insert(const pstring& source)
   {
      sources_.push_back(source);
   }

   void meta_target::insert(const std::vector<pstring>& srcs)
   {
      sources_.insert(sources_.end(), srcs.begin(), srcs.end());
   }

   void meta_target::instantiate_meta_targets(const meta_targets_t& targets, 
                                              const feature_set& build_request,
                                              std::vector<basic_target*>* result, 
                                              feature_set* usage_requirments) const
   {
      for(meta_targets_t::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
         (**i).instantiate(build_request, result, usage_requirments);
   }

   void meta_target::instantiate_simple_targets(const sources_t& targets, 
                                                const feature_set& build_request,
                                                const main_target& owner, 
                                                std::vector<basic_target*>* result) const
   {
      for(sources_t::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
      {
         const type* tp = project_->engine()->get_type_registry().resolve_from_target_name(*i);
         if (tp == 0)
            throw std::runtime_error("Can't resolve type from source '" + i->to_string() + "'.");

         source_target* st = new(project_->engine()) source_target(&owner, *i, tp, &owner.properties());
         result->push_back(st);
      }
   }
   
   void meta_target::instantiate(const feature_set& build_request,
                                 std::vector<basic_target*>* result, 
                                 feature_set* usage_requirements) const
   {
      feature_set* mt_fs = build_request.clone();
      requirements().eval(mt_fs, project()->engine()->feature_registry());
      project_->engine()->feature_registry().add_defaults(mt_fs);

      vector<basic_target*> sources;

      sources_t simple_targets;
      meta_targets_t meta_targets;
      split_sources(&simple_targets, &meta_targets);

      feature_set* build_request_with_propagated = build_request.clone();
      build_request_with_propagated->copy_propagated(*mt_fs);
      instantiate_meta_targets(meta_targets, *build_request_with_propagated, &sources, usage_requirements);
      
      mt_fs->join(*usage_requirements);
      main_target* mt = new(project_->engine()->targets_pool()) 
                           main_target(this, 
                                       name(), 
                                       instantiate_type(*mt_fs), 
                                       mt_fs,
                                       project_->engine()->targets_pool());

      instantiate_simple_targets(simple_targets, *mt_fs, *mt, &sources);
      
      mt->sources(sources);
      this->usage_requirements().eval(*mt_fs, usage_requirements);
      
      result->push_back(mt);
   }

   void meta_target::split_sources(sources_t* simple_targets, meta_targets_t* meta_targets) const
   {
      const type_registry& tr = project_->engine()->get_type_registry();
      for(sources_t::const_iterator i = sources_.begin(), last = sources_.end(); i != last; ++i)
      {
         if (const type* t = tr.resolve_from_target_name(*i))
            simple_targets->push_back(*i); 
         else
            resolve_meta_target_source(*i, meta_targets);
     }
   }

   // TODO: 
   // 1. Если подаем только директорию проекта и там есть две альтернативы, то по идее нужно было бы выбрать одну из них уже 
   //    на этом этапе ибо другой возможности у нас уже не будет.
   static boost::regex project_splitter("(.+?)(?://(.+))?");
   void meta_target::resolve_meta_target_source(const pstring& source, 
                                                meta_targets_t* meta_targets) const
   {
      boost::cmatch m;
      if (!boost::regex_match(source.begin(), source.end(), m, project_splitter))
         throw std::runtime_error("Can't parse meta target '" + source.to_string() + "'.");
      
      string target_name = m[1];
      if (const meta_target* m = project_->find_target(pstring(project_->engine()->pstring_pool(), target_name)))
      {
         meta_targets->push_back(m);
         return;
      }

      location_t target_path(project_->location()); 
      target_path /= target_name;
      const hammer::project& p = project_->engine()->load_project(target_path);
      for(hammer::project::targets_t::const_iterator i = p.targets().begin(), last = p.targets().end(); i != last; ++i)
         meta_targets->push_back(i->second);
   }
}