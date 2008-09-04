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
      const hammer::type* tp = project_->engine()->get_type_registry().resolve_from_target_name(i->target_name_);
      if (tp == 0)
         throw std::runtime_error("Can't resolve type from source '" + i->target_name_.to_string() + "'.");

      source_target* st = new(project_->engine()) source_target(&owner, i->target_name_, tp, &owner.properties());
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

void basic_meta_target::split_sources(sources_decl* simple_targets, meta_targets_t* meta_targets,
                                      const feature_set& build_request) const
{
   const type_registry& tr = project_->engine()->get_type_registry();
   for(sources_decl::const_iterator i = sources_.begin(), last = sources_.end(); i != last; ++i)
   {
      if (const type* t = tr.resolve_from_target_name(i->target_path_))
         simple_targets->push_back(i->target_path_); 
      else
         resolve_meta_target_source(*i, build_request, simple_targets, meta_targets);
  }
}

// TODO: 
// 1. ���� ������ ������ ���������� ������� � ��� ���� ��� ������������, �� �� ���� ����� ���� �� ������� ���� �� ��� ��� 
//    �� ���� ����� ��� ������ ����������� � ��� ��� �� �����.
//static boost::regex project_splitter("(.+?)(?://(.+))?");
void basic_meta_target::resolve_meta_target_source(const sources_decl::source_decl& source,
                                                   const feature_set& build_request,
                                                   sources_decl* simple_targets,
                                                   meta_targets_t* meta_targets) const
{
	if (source.target_name_.empty() && 
       !source.target_path_.empty())
	{
		if (const basic_meta_target* m = project_->find_target(source.target_name_))
		{
			m->transfer_sources(simple_targets, meta_targets, build_request);
			meta_targets->push_back(m);
			return;
		}
   }

	const hammer::project& p = project_->engine()->load_project(source.target_path_.to_string(), *project_);
	for(hammer::project::targets_t::const_iterator i = p.targets().begin(), last = p.targets().end(); i != last; ++i)
	{
		if (!i->second->is_explicit())
		{
			i->second->transfer_sources(simple_targets, meta_targets, build_request);
			meta_targets->push_back(i->second);
		}
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