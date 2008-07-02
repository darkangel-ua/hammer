#include "stdafx.h"
#include "project.h"
#include "engine.h"
#include "feature_registry.h"

using namespace std;

namespace hammer{

   project::project(hammer::engine* e, 
                    const pstring& name, 
                    const location_t& location, 
                    const requirements_decl& req,
                    const requirements_decl& usage_req)
                   :
                    basic_meta_target(this, name, req, usage_req), location_(location), engine_(e)
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

   const basic_meta_target*
   project::select_best_alternative(const std::string& target_name, 
                                    const feature_set& f) const
   {
      boost::iterator_range<targets_t::const_iterator> r = 
         targets_.equal_range(pstring(engine_->pstring_pool(), target_name));

      if (empty(r))
         throw std::runtime_error("Can't find target '" + target_name + "'");

      if (size(r) != 1)
         throw std::runtime_error("Can't select alternative yet :(");

      return r.begin()->second;
   }

   
   void project::instantiate(const std::string& target_name,
                             const feature_set& build_request,
                             std::vector<basic_target*>* result) const
   {
      const basic_meta_target* best_target = select_best_alternative(target_name, build_request);
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

}