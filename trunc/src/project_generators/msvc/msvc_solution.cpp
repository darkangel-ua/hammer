#include "stdafx.h"
#include "msvc_solution.h"
#include <boost/guid.hpp>
#include "msvc_project.h"
#include <boost/ptr_container/ptr_map.hpp>
#include <vector>
#include "../../main_target.h"
#include "../../engine.h"
#include <boost/filesystem/fstream.hpp>

using namespace std;

namespace hammer{ namespace project_generators{

namespace
{
   struct solution_t
   {
   };
}

struct msvc_solution::impl_t
{
   typedef msvc_project::dependencies_t dependencies_t;
   typedef boost::ptr_map<const meta_target*, msvc_project> projects_t;

   impl_t(engine& e) : engine_(e){}
   void generate_dependencies(dependencies_t::const_iterator first, 
                              dependencies_t::const_iterator last) const;

   engine& engine_;
   location_t output_location_;

   mutable projects_t projects_;
};

typedef msvc_solution::impl_t impl_t;

void impl_t::generate_dependencies(impl_t::dependencies_t::const_iterator first, 
                                   impl_t::dependencies_t::const_iterator last) const
{
   dependencies_t dependencies;
   for(; first != last; ++first)
   {
      projects_t::const_iterator i = projects_.find((**first).meta_target());
      if (i == projects_.end() ||
         (i != projects_.end() &&
          !i->second->has_variant(*first)))
      {
         
         auto_ptr<msvc_project> p_guard(new msvc_project(engine_));
         msvc_project* p = p_guard.get();
         projects_.insert(&p->meta_target(), p_guard);
         p->add_variant((**first).build_node());
         p->generate();
         dependencies.insert(dependencies.end(), p->dependencies().begin(), p->dependencies().end());
      }
   }

   if (!dependencies.empty())
   {
      std::sort(dependencies.begin(), dependencies.end());
      dependencies.erase(std::unique(dependencies.begin(), dependencies.end()), dependencies.end());
      generate_dependencies(dependencies.begin(), dependencies.end());
   }
}

msvc_solution::msvc_solution(engine& e) : impl_(new impl_t(e))
{
}

msvc_solution::~msvc_solution()
{
   delete impl_;
}

void msvc_solution::add_target(boost::intrusive_ptr<const build_node> node)
{
   std::auto_ptr<msvc_project> p_guarg(new msvc_project(impl_->engine_));
   msvc_project* p = p_guarg.get();
   typedef vector<const main_target*> dependencies_t;
   dependencies_t dependencies;
   p->add_variant(node);
   impl_->projects_.insert(&p->meta_target(), p_guarg);
   impl_->output_location_ = impl_->engine_.root() / p->meta_target().location().to_string();
}

void msvc_solution::write() const
{
   boost::filesystem::ofstream f;
   location_t filename = impl_->output_location_ / "dummy.sln";
   f.open(filename, std::ios::trunc);
   //throw std::runtime_error("Can't write '" + filename.string() + "'.";
   msvc_project::dependencies_t dependencies;
   typedef impl_t::projects_t::const_iterator iter;
   for(iter i = impl_->projects_.begin(), last = impl_->projects_.end(); i != last; ++i)
   {
      i->second->generate();
      dependencies.insert(dependencies.begin(), 
                          i->second->dependencies().begin(), 
                          i->second->dependencies().end());
   }

   impl_->generate_dependencies(dependencies.begin(), dependencies.end());
}

}}