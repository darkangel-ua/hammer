#include "stdafx.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "source_target.h"
#include "main_target.h"
#include "feature_set.h"

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

   std::vector<basic_target*> meta_target::instantiate(const feature_set& build_request) const
   {
       main_target* mt = new(project_->engine()->targets_pool()) 
                           main_target(this, 
                                       name(), 
                                       instantiate_type(), 
                                       features_->join(build_request),
                                       project_->engine()->targets_pool());

      vector<basic_target*> sources;
      for(sources_t::const_iterator i = sources_.begin(), last = sources_.end(); i != last; ++i)
      {
         if (const meta_target* t = project_->find_target(*i))
         {
            vector<basic_target*> r(t->instantiate(build_request));
            sources.insert(sources.end(), r.begin(), r.end());
         }
         else
         {
            const type* tp = project_->engine()->get_type_registry().resolve_from_target_name(*i, mt->features());
            source_target* st = new(project_->engine()) source_target(mt, *i, tp, &mt->features());
            sources.push_back(st);
         }
      }

      mt->sources(sources);
      
      vector<basic_target*> result;
      result.push_back(mt);
      
      return result;
   }

   std::vector<basic_target*> meta_target::generate() const
   {
      return std::vector<basic_target*>();
//      project_->engine()->generators().generate(*this);
   }

}