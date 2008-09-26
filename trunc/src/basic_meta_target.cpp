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
   // FIXME: нужно вынести этот код куда-то в другое место, так как это не единственное место, когда нужно установить данные для фич
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
      const hammer::type* tp = project_->engine()->get_type_registry().resolve_from_target_name(i->target_path_);
      if (tp == 0)
         throw std::runtime_error("Can't resolve type from source '" + i->target_path_.to_string() + "'.");

      source_target* st = new(project_->engine()) source_target(&owner, i->target_path_, tp, &owner.properties());
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

void basic_meta_target::split_one_source(sources_decl* simple_targets,
                                         meta_targets_t* meta_targets,
                                         const source_decl& source,
                                         const feature_set& build_request,
                                         const type_registry& tr) const
{
   if (const type* t = tr.resolve_from_target_name(source.target_path_))
      simple_targets->push_back(source.target_path_); 
   else
      resolve_meta_target_source(source, build_request, simple_targets, meta_targets);
}

void basic_meta_target::split_sources(sources_decl* simple_targets, meta_targets_t* meta_targets,
                                      const sources_decl& sources, const feature_set& build_request) const
{
   const type_registry& tr = project_->engine()->get_type_registry();
   for(sources_decl::const_iterator i = sources.begin(), last = sources.end(); i != last; ++i)
      split_one_source(simple_targets, meta_targets, *i, build_request, tr);
}      

static const basic_meta_target* select_best_alternative(const hammer::project& p, 
                                                        const pstring& target_name,
                                                        const feature_set& build_request,
                                                        const feature_set* target_features)
{
   if (target_features == NULL)
      return p.select_best_alternative(target_name, build_request);
   else
      return p.select_best_alternative(target_name, *build_request.join(*target_features));
}

// TODO: 
// 1. Если подаем только директорию проекта и там есть две альтернативы, то по идее нужно было бы выбрать одну из них уже 
//    на этом этапе ибо другой возможности у нас уже не будет.
void basic_meta_target::resolve_meta_target_source(const source_decl& source,
                                                   const feature_set& build_request,
                                                   sources_decl* simple_targets,
                                                   meta_targets_t* meta_targets) const
{
	// check that source is simple one ID. May be its source or may be target ID.
   if (source.target_name_.empty() && 
       !source.target_path_.empty())
	{
		if (const basic_meta_target* m = project_->find_target(source.target_path_))
		{
			m = select_best_alternative(*project_, source.target_path_, build_request, source.properties_);
         m->transfer_sources(simple_targets, meta_targets, build_request);
			meta_targets->push_back(m);
			return;
		}
   }

	// source has target_name_ only when it was explicitly requested (./foo//bar) where target_name_ == "bar"
   const hammer::project& target_project = project_->engine()->load_project(source.target_path_.to_string(), *project_);
   if (source.target_name_.empty()) 
   {
      hammer::project::selected_targets_t selected_targets(target_project.select_best_alternative(source.properties_ == NULL ? build_request : *build_request.join(*source.properties_)));
      for(hammer::project::selected_targets_t::const_iterator i = selected_targets.begin(), last = selected_targets.end(); i != last; ++i)
	   {
		   if (!(**i).is_explicit())
		   {
			   (**i).transfer_sources(simple_targets, meta_targets, build_request);
			   meta_targets->push_back(*i);
		   }
	   }
   }
   else
   {
      const basic_meta_target* m = select_best_alternative(target_project, source.target_name_, build_request, source.properties_);
      m->transfer_sources(simple_targets, meta_targets, build_request);
      meta_targets->push_back(m);
      return;
   }
}

void basic_meta_target::transfer_sources(sources_decl* simple_targets, 
                                         meta_targets_t* meta_targets,
                                         const feature_set& build_request) const
{

}

basic_meta_target::~basic_meta_target()
{
}

}