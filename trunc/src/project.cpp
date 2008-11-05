#include "stdafx.h"
#include "project.h"
#include "engine.h"
#include "feature_registry.h"
#include "feature_set.h"
#include "feature.h"

using namespace std;

namespace hammer{

   project::project(hammer::engine* e, 
                    const pstring& name, 
                    const location_t& location, 
                    const requirements_decl& req,
                    const requirements_decl& usage_req)
                   :
                    basic_meta_target(this, name, req, usage_req), location_(location), engine_(e),
                    is_root_(false)
   {
   }

  void project::add_target(std::auto_ptr<basic_meta_target> t)
  {
     targets_.insert(t->name(), t.get());
     t.release();
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

   static bool is_alternative_suitable(const feature_set& target_properties, 
                                       const feature_set& build_request)
   {
      for(feature_set::const_iterator i = target_properties.begin(), last = target_properties.end(); i != last; ++i)
      {
         if (!((**i).attributes().free ||
               (**i).attributes().incidental))
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

   const basic_meta_target* 
   project::select_best_alternative(const pstring& target_name, 
                                    const feature_set& build_request) const
   {
      const basic_meta_target* result = try_select_best_alternative(target_name, build_request);
      if (result == NULL)
         throw std::runtime_error("Can't select alternative for target '" + target_name.to_string() + "'.");

      return result;
   }

   const basic_meta_target* 
   project::try_select_best_alternative(const pstring& target_name, 
                                        const feature_set& build_request) const
   {
      boost::iterator_range<targets_t::const_iterator> r = targets_.equal_range(target_name);

      if (r.empty())
         throw std::runtime_error("Can't find target '" + target_name.to_string() + "'");

      const basic_meta_target* result = NULL;
      
      for(targets_t::const_iterator first = begin(r), last = end(r); first != last; ++first)
      {
         feature_set* fs = engine_->feature_registry().make_set();
         first->second->requirements().eval(build_request, fs);
         if (is_alternative_suitable(*fs, build_request))
         {
            if (result != NULL)
               throw std::runtime_error("Can't select alternative for target '" + target_name.to_string() + "' between others[fixme]");

            result = first->second;
         }
      }

      return result;
   }

   
   void project::instantiate(const std::string& target_name,
                             const feature_set& build_request,
                             std::vector<basic_target*>* result) const
   {
      const basic_meta_target* best_target = select_best_alternative(pstring(engine_->pstring_pool(), target_name), build_request);
      feature_set* usage_requirements = engine_->feature_registry().make_set();
      best_target->instantiate(0, build_request, result, usage_requirements);
   }
   
   void project::instantiate(const main_target* owner, 
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
         const basic_meta_target* t = try_select_best_alternative(first->second->name(), build_request);
         if (t != NULL)
            result.push_back(t);
         
         // skip meta targets with equal names
         targets_t::const_iterator next = first; std::advance(next, 1);
         while(next != last && first->first == next->first)
            ++first, ++next;
         
         first = next;
      }

      return result;
   }
}