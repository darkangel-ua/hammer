#include "stdafx.h"
#include "msvc_project_generator.h"
#include "main_target.h"
#include "feature_set.h"
#include <vector>
#include "project_generators/msvc/msvc_project.h"
#include "types.h"
#include "engine.h"
#include "type_registry.h"
#include <iostream>

using namespace std;
namespace hammer{ namespace project_generators{

   struct msvc::impl_t
   {
      impl_t(engine& e) : engine_(e)
      {
         exe_type_ = e.get_type_registry().resolve_from_name(types::EXE.name());
         shared_lib_type_ = e.get_type_registry().resolve_from_name(types::SHARED_LIB.name());
         cpp_type_ = e.get_type_registry().resolve_from_name(types::CPP.name());
         h_type_ = e.get_type_registry().resolve_from_name(types::H.name());
      }

      void walk_over_targets(const main_target* t, 
                             const feature_set& props);
      
      const engine& engine_;
      typedef vector<pair<vector<basic_target*>, const feature_set*> > target_variants_t;
      typedef boost::ptr_map<const pstring, msvc_project> projects_t;

      target_variants_t target_variants_;
      projects_t projects_;
      const type* exe_type_;
      const type* shared_lib_type_;
      const type* cpp_type_;
      const type* h_type_;
   };

   typedef msvc::impl_t impl_t;

   msvc::msvc(engine& e) : impl_(new impl_t(e))
   {
   }

   msvc::~msvc()
   {
      delete impl_;
   }

   void impl_t::walk_over_targets(const main_target* t, 
                                  const feature_set& props)
   {
      projects_t::iterator p = projects_.find(t->name());
      if (p == projects_.end())
      {
         auto_ptr<msvc_project> new_project(new msvc_project);
         new_project->add_variant(t);
         projects_.insert(t->name(), new_project.get());
         new_project.release();
      }

      typedef main_target::sources_t::const_iterator iter;
      for(iter i = t->sources().begin(), last = t->sources().end(); i != last; ++i)
      {
         const type* tp = (**i).type();
         if (tp == exe_type_ ||
             tp == shared_lib_type_)
         {
            walk_over_targets(static_cast<const main_target*>(*i), props);
         }
      }
   }

   void msvc::add_variant(const std::vector<basic_target*>& targets, 
                          const feature_set& variant)
   {
      impl_->target_variants_.push_back(make_pair(targets, &variant));
   }

   void msvc::generate()
   {
      for(impl_t::target_variants_t::const_iterator i = impl_->target_variants_.begin(), last = impl_->target_variants_.end(); i != last; ++i)
         for(vector<basic_target*>::const_iterator j = i->first.begin(), j_last = i->first.end(); j != j_last; ++j)
            impl_->walk_over_targets(dynamic_cast<const main_target*>(*j), *i->second);
   
      for(impl_t::projects_t::iterator i = impl_->projects_.begin(), last = impl_->projects_.end(); i != last; ++i)
         i->second->generate();
   }
}}
