#include "stdafx.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "source_target.h"
#include "main_target.h"

using namespace std;

namespace hammer{
   meta_target::meta_target(hammer::project* p, const pstring& name, const type* t) 
      : project_(p), name_(name), type_(t)
   {

   }

   void meta_target::insert(const pstring& t)
   {
      targets_.push_back(t);
   }

   std::vector<basic_target*> meta_target::instantiate(const feature_set& build_request) const
   {
       main_target* mt = new(project_->engine()->targets_pool()) 
                           main_target(this, 
                                       name(), 
                                       project_->engine()->get_type_registry().resolve(type_, build_request), 
                                       &build_request,
                                       project_->engine()->targets_pool());

      vector<basic_target*> sources;
      for(targets_t::const_iterator i = targets_.begin(), last = targets_.end(); i != last; ++i)
      {
         if (const meta_target* t = project_->find_target(*i))
         {
            vector<basic_target*> r(t->instantiate(build_request));
            sources.insert(sources.end(), r.begin(), r.end());
         }
         else
         {
            const type* tp = project_->engine()->get_type_registry().resolve_from_target_name(*i, build_request);
            source_target* st = new(project_->engine()) source_target(mt, *i, tp);
            sources.push_back(st);
         }
      }

      mt->sources(sources);
      
      vector<basic_target*> result;
      result.push_back(mt);
      
      return result;
   }
}