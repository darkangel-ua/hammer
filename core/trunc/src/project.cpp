#include "stdafx.h"
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>

using namespace std;

namespace hammer{

project::project(hammer::engine* e, 
                 const pstring& name, 
                 const location_t& location, 
                 const requirements_decl& req,
                 const requirements_decl& usage_req)
                :
                 basic_meta_target(this, name, req, usage_req), location_(location), engine_(e),
                 is_root_(false),
                 add_targets_as_explicit_(false),
                 local_feature_registry_(&pool_for_feature_registry_)
{
}


void project::add_target(std::auto_ptr<basic_meta_target> t)
{
  if (add_targets_as_explicit_)
   t->set_explicit(true);

  targets_.insert(t->name(), t.get());
  t.release();
}

void project::add_targets_as_explicit(bool v) 
{ 
   add_targets_as_explicit_ = v; 
}

const basic_meta_target* project::find_target(const pstring& name) const
{
   return const_cast<project*>(this)->find_target(name);
}

basic_meta_target* project::find_target(const pstring& name)
{
   targets_t::iterator i = targets_.find(name);
   if (i == targets_.end())
      return 0;
   else
      return i->second;
}

bool is_alternative_suitable(const feature_set& target_properties, 
                             const feature_set& build_request)
{
   for(feature_set::const_iterator i = target_properties.begin(), last = target_properties.end(); i != last; ++i)
   {
      if (!((**i).attributes().free ||
            (**i).attributes().incidental ||
            (**i).attributes().undefined_))
      {
         feature_set::const_iterator f = build_request.find((**i).name());
         if (f != build_request.end() && 
             (**i).value() != (**f).value())
         {
            return false;
         }
      }
   }

   return true;
}

project::selected_target
project::select_best_alternative(const pstring& target_name, 
                                 const feature_set& build_request) const
{
   
   selected_target result = try_select_best_alternative(target_name, build_request);
   if (result.target_ == NULL)
      throw std::runtime_error("Can't select alternative for target '" + target_name.to_string() + "'.");

   return result;
}

project::selected_target
project::try_select_best_alternative(const pstring& target_name, 
                                     const feature_set& build_request_param) const
{
   const feature_set& build_request = build_request_param.has_undefined_features()
                                         ? *try_resolve_local_features(build_request_param) 
                                         : build_request_param;         

   boost::iterator_range<targets_t::const_iterator> r = targets_.equal_range(target_name);

   if (r.empty())
      throw std::runtime_error("Can't find target '" + target_name.to_string() + "'");

   selected_target result;
   bool overriden = false;

   for(targets_t::const_iterator first = begin(r), last = end(r); first != last; ++first)
   {
      feature_set* fs = engine_->feature_registry().make_set();
      first->second->requirements().eval(build_request, fs);
      if (is_alternative_suitable(*fs, build_request))
      {
         if (result.target_ != NULL)
         {
            feature_set::const_iterator override_iter = fs->find("override");
            if (overriden && override_iter != fs->end() ||
                !overriden && override_iter == fs->end())
            {
               throw std::runtime_error("Can't select alternative for target '" + target_name.to_string() + "' between others[fixme]");
            }
         }

         result.target_ = first->second;
         result.resolved_build_request_ = &build_request;
      }
   }

   return result;
}


void project::instantiate(const std::string& target_name,
                          const feature_set& build_request,
                          std::vector<basic_target*>* result) const
{
   selected_target best_target = select_best_alternative(pstring(engine_->pstring_pool(), target_name), build_request);
   feature_set* usage_requirements = engine_->feature_registry().make_set();
   best_target.target_->instantiate(0, *best_target.resolved_build_request_, result, usage_requirements);
}

void project::instantiate_impl(const main_target* owner, 
                               const feature_set& build_request,
                               std::vector<basic_target*>* result, 
                               feature_set* usage_requirements) const
{
   assert(false && "not implemented.");
}

bool project::operator == (const project& rhs) const
{
   return this == &rhs;
}

project::selected_targets_t 
project::select_best_alternative(const feature_set& build_request) const
{
   selected_targets_t result;
   
   targets_t::const_iterator first = targets_.begin(), last = targets_.end();
   while(first != last)
   {
      selected_target t = try_select_best_alternative(first->second->name(), build_request);
      if (t.target_ != NULL)
         result.push_back(t);
      
      // skip meta targets with equal names
      targets_t::const_iterator next = first; std::advance(next, 1);
      while(next != last && first->first == next->first)
         ++first, ++next;
      
      first = next;
   }

   return result;
}

feature_set* project::try_resolve_local_features(const feature_set& fs) const
{
   feature_set* result = engine_->feature_registry().make_set();
   for(feature_set::const_iterator i = fs.begin(), last = fs.end(); i != last; ++i)
   {
      if ((**i).attributes().undefined_)
      {
         const feature_def* def = local_feature_registry_.find_def_from_full_name((**i).name().c_str());
         if (def != NULL)
            result->join(local_feature_registry_.create_feature((**i).name(), (**i).value().to_string()));
         else
            result->join(*i);
      }
      else
         result->join(*i);
   }

   return result;
}

}