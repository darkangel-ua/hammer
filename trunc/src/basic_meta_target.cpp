#include "stdafx.h"
#include "basic_meta_target.h"
#include "feature_set.h"
#include "feature.h"
#include "requirements_decl.h"
#include "type.h"
#include "project.h"
#include "engine.h"
#include "type_registry.h"
#include "source_target.h"
#include "main_target.h"
#include <boost/regex.hpp>

using namespace std;

namespace hammer{

basic_meta_target::basic_meta_target(hammer::project* p,
                                     const pstring& name, 
                                     const requirements_decl& req, 
                                     const requirements_decl& usage_req)
                                     : 
                                     project_(p),
                                     name_(name),
                                     requirements_(req),
                                     usage_requirements_(usage_req),
                                     is_explicit_(false)
{
   requirements_.setup_path_data(this);
   usage_requirements_.setup_path_data(this);
}

void basic_meta_target::insert(const pstring& source)
{
   sources_.push_back(source);
}

void basic_meta_target::sources(const sources_decl& s)
{
   sources_ = s;
}

const location_t& basic_meta_target::location() const 
{
   return project()->location();
}

void basic_meta_target::instantiate_simple_targets(const sources_decl& targets, 
                                                   const feature_set& build_request,
                                                   const main_target& owner, 
                                                   std::vector<basic_target*>* result) const
{
   for(sources_decl::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
   {
      const hammer::type* tp = project_->engine()->get_type_registry().resolve_from_target_name(*i);
      if (tp == 0)
         throw std::runtime_error("Can't resolve type from source '" + i->to_string() + "'.");

      source_target* st = new(project_->engine()) source_target(&owner, *i, tp, &owner.properties());
      result->push_back(st);
   }
}

void basic_meta_target::instantiate_meta_targets(const meta_targets_t& targets, 
                                                 const feature_set& build_request,
                                                 const main_target* owner, 
                                                 std::vector<basic_target*>* result, 
                                                 feature_set* usage_requirments) const
{
   for(meta_targets_t::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
      (**i).instantiate(owner, build_request, result, usage_requirments);
}

void basic_meta_target::split_sources(sources_decl* simple_targets, meta_targets_t* meta_targets) const
{
   const type_registry& tr = project_->engine()->get_type_registry();
   for(sources_decl::const_iterator i = sources_.begin(), last = sources_.end(); i != last; ++i)
   {
      if (const type* t = tr.resolve_from_target_name(*i))
         simple_targets->push_back(*i); 
      else
         resolve_meta_target_source(*i, simple_targets, meta_targets);
  }
}

// TODO: 
// 1. Если подаем только директорию проекта и там есть две альтернативы, то по идее нужно было бы выбрать одну из них уже 
//    на этом этапе ибо другой возможности у нас уже не будет.
static boost::regex project_splitter("(.+?)(?://(.+))?");
void basic_meta_target::resolve_meta_target_source(const pstring& source,
                                                   sources_decl* simple_targets,
                                                   meta_targets_t* meta_targets) const
{
   boost::cmatch m;
   if (!boost::regex_match(source.begin(), source.end(), m, project_splitter))
      throw std::runtime_error("Can't parse meta target '" + source.to_string() + "'.");
   
   string target_name = m[1];
   if (const basic_meta_target* m = project_->find_target(pstring(project_->engine()->pstring_pool(), target_name)))
   {
      m->transfer_sources(simple_targets, meta_targets);
      meta_targets->push_back(m);
      return;
   }

   location_t target_path(project_->location()); 
   target_path /= target_name;
   const hammer::project& p = project_->engine()->load_project(target_path);
   for(hammer::project::targets_t::const_iterator i = p.targets().begin(), last = p.targets().end(); i != last; ++i)
   {
      if (!i->second->is_explicit())
      {
         i->second->transfer_sources(simple_targets, meta_targets);
         meta_targets->push_back(i->second);
      }
   }
}

void basic_meta_target::transfer_sources(sources_decl* simple_targets, 
                                         meta_targets_t* meta_targets) const
{

}

basic_meta_target::~basic_meta_target()
{
}

}