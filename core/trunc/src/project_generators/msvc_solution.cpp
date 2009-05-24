#include "stdafx.h"
#include <set>
#include <vector>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/guid.hpp>
#include <hammer/core/engine.h>
#include <hammer/core/feature.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/project_generators/msvc_project.h>
#include <hammer/core/project_generators/msvc_solution.h>

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
   typedef std::vector<std::string> variant_names_t;

   impl_t(msvc_solution* owner, const project& source_project, 
          const location_t& output_path,
          generation_mode::value mode) 
      : owner_(owner), 
        source_project_(source_project),
        engine_(*source_project.get_engine()), 
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
   std::vector<std::string> variant_names_;

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
      projects_t::const_iterator i = projects_.find((**first).get_meta_target());
      if (i == projects_.end() ||
         (i != projects_.end() &&
          !i->second->has_variant(*first)))
      {
         
         auto_ptr<msvc_project> p_guard(new msvc_project(engine_, project_output_dir(*(**first).build_node()), variant_names_.front(), owner_->generate_id()));
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
   msvc_project::dependencies_t sorted_dependencies(project.dependencies());
   // stabilize order to allow normal testing. May be FIXME:
   std::sort(sorted_dependencies.begin(), sorted_dependencies.end(), &less_by_location_and_name);

   typedef msvc_project::dependencies_t::const_iterator iter;
   for(iter i = sorted_dependencies.begin(), last = sorted_dependencies.end(); i != last; ++i)
   {
      projects_t::const_iterator p = projects_.find((**i).get_meta_target());
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
         location_t result = node.products_[0]->get_main_target()->location() / suffix;
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
   if (!impl_->projects_.empty())
      throw std::runtime_error("MSVC solution generator can handle only one top level target.");

   impl_->variant_names_.push_back(node->products_[0]->get_main_target()->properties().get("variant").value().to_string());
   std::auto_ptr<msvc_project> p_guarg(new msvc_project(impl_->engine_, 
                                                        impl_->project_output_dir(*node), 
                                                        impl_->variant_names_.front(), 
                                                        generate_id()));
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
   for(sorted_projects_t::const_iterator i = sorted_projects.begin(), last = sorted_projects.end(); i != last; ++i)
   {
      (**i).write();
      impl_->write_project_section(f, **i);
   }
   
   f << "Global\n"
     << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n";
   
   for(impl_t::variant_names_t::const_iterator i = impl_->variant_names_.begin(), last = impl_->variant_names_.end(); i != last; ++i)
      f << "\t\t" << *i << "|Win32" << " = " << *i << "|Win32\n";
   
   f << "\tEndGlobalSection\n" 
     << "EndGlobal\n";
}

boost::guid msvc_solution::generate_id() const
{
   return impl_->generate_id();
}

}}