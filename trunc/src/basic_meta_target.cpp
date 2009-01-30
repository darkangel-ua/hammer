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
#include "feature_registry.h"

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

void basic_meta_target::sources(const sources_decl& s)
{
   sources_ = s;
}

void basic_meta_target::add_sources(const sources_decl& s)
{
   sources_.insert(s);
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
   const type_registry& tr = project_->engine()->get_type_registry();
   for(sources_decl::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
   {
      const hammer::type* tp = i->type();
      if (tp == 0)
         throw std::runtime_error("Can't resolve type from source '" + i->target_path().to_string() + "'.");

      source_target* st = new(project_->engine()) source_target(&owner, i->target_path(), tp, &owner.properties());
      result->push_back(st);
   }
}

void basic_meta_target::instantiate_meta_targets(const meta_targets_t& targets, 
                                                 const feature_set& build_request,
                                                 const main_target* owner, 
                                                 std::vector<basic_target*>* result, 
                                                 feature_set* usage_requirments) const
{
   // FIXME: если в result уже есть проинстанцированная цель то нужно проверить с какими параметрами это было сделанно
   // Если они совпадают значит мы просто пропускаем инстанцирование, если нет, то кидаем исключение
   for(meta_targets_t::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i)
   {
      i->first->instantiate(owner, i->second == NULL ? build_request : *build_request.join(*i->second), 
                            result, usage_requirments);
   }
}

void basic_meta_target::split_one_source(sources_decl* simple_targets,
                                         meta_targets_t* meta_targets,
                                         const source_decl& source,
                                         const feature_set& build_request,
                                         const type_registry& tr) const
{
   if (const type* t = source.type())
      simple_targets->push_back(source); 
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

void basic_meta_target::resolve_meta_target_source(const source_decl& source,
                                                   const feature_set& build_request,
                                                   sources_decl* simple_targets,
                                                   meta_targets_t* meta_targets) const
{
   const feature_set* build_request_with_source_properties = (source.properties() == NULL ? &build_request : build_request.join(*source.properties()));

	// check that source is simple one ID. Maybe its source or maybe its target ID.
   if (source.target_name().empty() && 
       !source.target_path().empty())
	{
		if (const basic_meta_target* m = project_->find_target(source.target_path()))
		{
         project::selected_target selected_target = project_->select_best_alternative(source.target_path(), *build_request_with_source_properties);
         selected_target.target_->transfer_sources(simple_targets, 
                                                   meta_targets, 
                                                   *selected_target.resolved_build_request_, 
                                                   source.properties());
			return;
		}
   }

	// source has target_name_ only when it was explicitly requested (./foo//bar) where target_name_ == "bar"
   engine::loaded_projects_t suitable_projects = project_->engine()->load_project(source.target_path().to_string(), *project_);
   if (source.target_name().empty()) 
   {
      try
      {
         hammer::project::selected_targets_t selected_targets(suitable_projects.select_best_alternative(*build_request_with_source_properties));
         for(hammer::project::selected_targets_t::const_iterator i = selected_targets.begin(), last = selected_targets.end(); i != last; ++i)
	      {
		      i->target_->transfer_sources(simple_targets, 
                                         meta_targets, 
                                         *i->resolved_build_request_, 
                                         source.properties());
	      }
      }
      catch(const std::exception& e)
      {
         throw std::runtime_error("While resolving meta target '" + source.target_path().to_string() + 
                                  "' at '" + location().native_file_string() + "\n" + e.what());
      }
   }
   else
   {
      project::selected_target selected_target = suitable_projects.select_best_alternative(source.target_name(), 
                                                                                           *build_request_with_source_properties);
      selected_target.target_->transfer_sources(simple_targets, meta_targets, 
                                                *selected_target.resolved_build_request_,
                                                source.properties());
   }
}

void basic_meta_target::transfer_sources(sources_decl* simple_targets, 
                                         meta_targets_t* meta_targets,
                                         const feature_set& build_request,
                                         const feature_set* additional_build_properties) const
{
   meta_targets->push_back(make_pair(this, additional_build_properties));
}

basic_meta_target::~basic_meta_target()
{
}

const feature_set& basic_meta_target::resolve_undefined_features(const feature_set& fs) const
{
   const feature_set* without_undefined = fs.has_undefined_features() 
                                             ? project()->try_resolve_local_features(fs)
                                             : &fs;
   if (without_undefined->has_undefined_features())
      throw std::runtime_error("Target '" + name().to_string() + "' at location '" +
                               location().native_file_string() + "' has been instantiated with unknown local features");

   return *without_undefined;
}

void basic_meta_target::instantiate(const main_target* owner, 
                                    const feature_set& build_request,
                                    std::vector<basic_target*>* result, 
                                    feature_set* usage_requirements) const
{
   if (is_cachable(owner))
   {
      for(instantiation_cache_t::const_iterator i = instantiation_cache_.begin(), last = instantiation_cache_.end(); i != last; ++i)
         if (*i->build_request_ == build_request)
         {
            result->insert(result->end(), i->instantiated_targets_.begin(), i->instantiated_targets_.end());
            usage_requirements->join(*i->computed_usage_requirements_);
            return;
         }

      cached_instantiation_data_t cache_item;
      cache_item.build_request_ = &build_request;
      cache_item.computed_usage_requirements_ = project()->engine()->feature_registry().make_set();

      instantiate_impl(owner, 
                       build_request, 
                       &cache_item.instantiated_targets_, 
                       cache_item.computed_usage_requirements_);
      instantiation_cache_.push_back(cache_item);
      
      result->insert(result->end(), cache_item.instantiated_targets_.begin(), cache_item.instantiated_targets_.end());
      usage_requirements->join(*cache_item.computed_usage_requirements_);
      
      return;
   }

   instantiate_impl(owner, 
                    build_request, 
                    result, 
                    usage_requirements);
}

}