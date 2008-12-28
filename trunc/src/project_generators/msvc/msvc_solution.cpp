#include "stdafx.h"
#include "msvc_solution.h"
#include <boost/guid.hpp>
#include "msvc_project.h"
#include <boost/ptr_container/ptr_map.hpp>
#include <vector>
#include "../../main_target.h"
#include "../../engine.h"
#include "../../meta_target.h"
#include "../../fs_helpers.h"
#include <boost/filesystem/fstream.hpp>
#include <set>

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

   impl_t(msvc_solution* owner, const project& source_project, 
          const location_t& output_path,
          generation_mode::value mode) 
      : owner_(owner), 
        source_project_(source_project),
        engine_(*source_project.engine()), 
        output_location_(output_path),
        generation_mode_(mode)
   {
      output_location_.normalize();
   }

   void generate_dependencies(dependencies_t::const_iterator first, 
                              dependencies_t::const_iterator last) const;
   void write_project_section(ostream& os, const msvc_project& project) const;
   boost::guid generate_id() const { return boost::guid::create(); }
   location_t project_output_dir(const build_node& node) const;

   msvc_solution* owner_;
   const project& source_project_;
   engine& engine_;
   location_t output_location_;
   std::string name_; // solution name;
   generation_mode::value generation_mode_;

   mutable projects_t projects_;
};

typedef msvc_solution::impl_t impl_t;

static bool less_by_location_and_name(const main_target* lhs, const main_target* rhs)
{
   location_t lhs_id = lhs->location() / lhs->name().to_string();
   location_t rhs_id = rhs->location() / rhs->name().to_string();
   lhs_id.normalize();
   rhs_id.normalize();

   return lhs_id < rhs_id;
}

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
         
         auto_ptr<msvc_project> p_guard(new msvc_project(engine_, project_output_dir(*(**first).build_node()), owner_->generate_id()));
         msvc_project* p = p_guard.get();
         p->add_variant((**first).build_node());
         p->generate();
         dependencies.insert(dependencies.end(), p->dependencies().begin(), p->dependencies().end());
         projects_.insert(&p->meta_target(), p_guard);
      }
   }

   if (!dependencies.empty())
   {
      std::sort(dependencies.begin(), dependencies.end());
      dependencies.erase(std::unique(dependencies.begin(), dependencies.end()), dependencies.end());
      generate_dependencies(dependencies.begin(), dependencies.end());
      // stabilize order to allow normal testing. May be FIXME:
      std::sort(dependencies.begin(), dependencies.end(), &less_by_location_and_name);
   }
}

// FIXME: при генерации солюшена может произойти хохма
// lib a : a.cpp ;
// lib b : a/<link>static b.cpp ;
// exe xoxma : main.cpp a b ;
//
// в этом случае имеем две разные конфигурации проекта в одной конфигурации солюшена чего студия не позволяет
 

void msvc_solution::impl_t::write_project_section(ostream& os, const msvc_project& project) const
{
   location_t project_path(relative_path(project.full_project_name(), output_location_));
   project_path.normalize();

   os << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" << project.name() 
      << "\", \"" << project_path << "\", \"{" << project.guid() << "}\"\n" ;

   os << "\tProjectSection(ProjectDependencies) = postProject\n";
   typedef msvc_project::dependencies_t::const_iterator iter;
   for(iter i = project.dependencies().begin(), last = project.dependencies().end(); i != last; ++i)
   {
      projects_t::const_iterator p = projects_.find((**i).meta_target());
      if (p == projects_.end())
         throw std::logic_error("Found target that is unknown to solution.");
      os << "\t\t{" << p->second->guid() << "} = {" << p->second->guid() << "}\n";
   }

   os << "\tEndProjectSection\n"
      << "EndProject\n";
}

location_t msvc_solution::impl_t::project_output_dir(const build_node& node) const
{
   switch(generation_mode_)
   {
      case generation_mode::LOCAL:
         return output_location_;

      case generation_mode::NON_LOCAL:
      {
         location_t suffix = relative_path(output_location_, source_project_.location());
         location_t result = node.products_[0]->mtarget()->location() / suffix;
         result.normalize();
         return result;
      }

      default:
         throw std::runtime_error("[msvc-solution] Unknown generation mode.");
   }
}

msvc_solution::msvc_solution(const project& source_project, const location_t& output_path,
                             generation_mode::value mode) 
  : impl_(new impl_t(this, source_project, output_path / "vc80", mode))
{
}

msvc_solution::~msvc_solution()
{
   delete impl_;
}

void msvc_solution::add_target(boost::intrusive_ptr<const build_node> node)
{
   std::auto_ptr<msvc_project> p_guarg(new msvc_project(impl_->engine_, impl_->project_output_dir(*node), generate_id()));
   msvc_project* p = p_guarg.get();
   p->add_variant(node);
   impl_->projects_.insert(&p->meta_target(), p_guarg);
   impl_->name_ = p->meta_target().name().to_string();

   msvc_project::dependencies_t dependencies;
   p->generate();
   dependencies.insert(dependencies.begin(), 
                       p->dependencies().begin(), 
                       p->dependencies().end());

   // stabilize order to allow normal testing. May be FIXME:
   std::sort(dependencies.begin(), dependencies.end(), &less_by_location_and_name);
   impl_->generate_dependencies(dependencies.begin(), dependencies.end());
}

static bool less_by_name(const msvc_project* lhs, 
                         const msvc_project* rhs)
{
   return lhs->name() < rhs->name();
}

void msvc_solution::write() const
{
   boost::filesystem::ofstream f;
   location_t filename = impl_->output_location_ / (impl_->name_ + ".sln");
   create_directories(filename.branch_path());
   f.open(filename, std::ios::trunc);
   //throw std::runtime_error("Can't write '" + filename.string() + "'.";
   f << "Microsoft Visual Studio Solution File, Format Version 9.00\n"
        "# Visual Studio 2005\n";
   
   // стабилизируем порядок проектов с солюшине, а то он все время меняется и 
   // невозможно нормально это тестировать
   typedef impl_t::projects_t::const_iterator iter;
   typedef vector<const msvc_project*> sorted_projects_t;

   sorted_projects_t sorted_projects;
   for(iter i = impl_->projects_.begin(), last = impl_->projects_.end(); i != last; ++i)
      sorted_projects.push_back(i->second);

   sort(sorted_projects.begin(), sorted_projects.end(), less_by_name);
   set<string> variant_names;
   for(sorted_projects_t::const_iterator i = sorted_projects.begin(), last = sorted_projects.end(); i != last; ++i)
   {
      (**i).write();
      impl_->write_project_section(f, **i);
      typedef msvc_project::variants_t::const_iterator viter;
      for(viter v = (*i)->variants().begin(), vlast = (*i)->variants().end(); v != vlast; ++v)
         variant_names.insert(v->name_);
   }
   
   f << "Global\n"
     << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n";
   for(set<string>::const_iterator i = variant_names.begin(), last = variant_names.end(); i != last; ++i)
      f << "\t\t" << *i << "|Win32" << " = " << *i << "|Win32\n";
   f << "EndGlobalSection\n";
}

boost::guid msvc_solution::generate_id() const
{
   return impl_->generate_id();
}

}}