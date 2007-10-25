#include "stdafx.h"
#include "meta_target.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "source_target.h"

using namespace std;

namespace hammer{
   meta_target::meta_target(project* p, const pstring& name, const type* t) 
      : project_(p), name_(name), type_(t)
   {

   }

   void meta_target::insert(const pstring& t)
   {
      targets_.push_back(t);
   }

   std::vector<basic_target*> meta_target::instantiate(const feature_set& build_request) const
   {
      vector<basic_target*> sources;
      for(targets_t::const_iterator i = targets_.begin(), last = targets_.end(); i != last; ++i)
      {
         if (meta_target* t = project_->find_target(*i))
         {
            vector<basic_target*> r(t->instantiate(build_request));
            sources.insert(result.end(), r.begin(), r.end());
         }
         else
         {
            const type* tp = project_->get_engine()->get_type_registry().resolve_from_target_name(*i, build_request);
            source_target* st = new(project_->get_engine()) source_target(this, *i, tp);
            sources.push_back(st);
         }
      }

      main_target* mt = new main_target(name(), type());
      mt->sources(sources);
      return result;
   }
}