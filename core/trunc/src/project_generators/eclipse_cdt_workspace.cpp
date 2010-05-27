#include "stdafx.h"
#include <hammer/core/project_generators/eclipse_cdt_workspace.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_unordered_map.hpp>
#include <hammer/core/project_generators/eclipse_cdt_project.h>
#include <hammer/core/project.h>
#include <hammer/core/main_target.h>
#include <boost/filesystem/convenience.hpp>

using namespace std;

namespace hammer{ namespace project_generators{

struct eclipse_cdt_workspace::impl_t
{
   typedef boost::ptr_unordered_map<const project*, eclipse_cdt_project> projects_t;
   typedef boost::unordered_set<const build_node*> visited_nodes_t;
   typedef boost::unordered_map<const project*, eclipse_cdt_project::project_main_targets_t> projects_main_targets_t;

   impl_t(const location_t& output_path, 
          const build_nodes_t& nodes,
          const location_t& templates_dir,
          eclipse_cdt_workspace& owner) 
      : output_path_(output_path),
        nodes_(nodes),
        templates_dir_(templates_dir),
        owner_(owner)
   {}

   void fill_projects(const build_node& node, 
                      visited_nodes_t& visited_nodes);

   const location_t output_path_; 
   build_nodes_t nodes_;
   location_t templates_dir_;
   projects_t projects_;
   projects_main_targets_t projects_main_targets_;
   eclipse_cdt_workspace& owner_;
};

eclipse_cdt_workspace::eclipse_cdt_workspace(const location_t& output_path, 
                                             const build_nodes_t& nodes,
                                             const location_t& templates_dir)
   : impl_(new impl_t(output_path, nodes, templates_dir, *this)) 
{

}

const location_t& eclipse_cdt_workspace::get_output_path() const
{
   return impl_->output_path_;
}

const location_t& eclipse_cdt_workspace::get_templates_dir() const
{
   return impl_->templates_dir_;
}

void eclipse_cdt_workspace::write() const
{
   create_directories(get_output_path());

   for(impl_t::projects_t::iterator i = impl_->projects_.begin(), last = impl_->projects_.end(); i != last; ++i)
      i->second->write();
}

void eclipse_cdt_workspace::construct()
{
   impl_->projects_.clear();
   impl_->projects_main_targets_.clear();
   impl_t::visited_nodes_t visited_nodes;
   for(build_nodes_t::const_iterator i = impl_->nodes_.begin(), last = impl_->nodes_.end(); i != last; ++i)
      impl_->fill_projects(**i, visited_nodes);

   for(impl_t::projects_t::iterator i = impl_->projects_.begin(), last = impl_->projects_.end(); i != last; ++i)
      i->second->construct(impl_->projects_main_targets_[i->first]);
}

eclipse_cdt_workspace::~eclipse_cdt_workspace()
{
}

void eclipse_cdt_workspace::impl_t::fill_projects(const build_node& node, 
                                                  visited_nodes_t& visited_nodes)
{
   if (visited_nodes.find(&node) != visited_nodes.end())
      return;

   for(build_node::sources_t::const_iterator i = node.sources_.begin(), last = node.sources_.end(); i != last; ++i)
   {
      if (visited_nodes.find(i->source_node_.get()) == visited_nodes.end())
      {
         visited_nodes.insert(i->source_node_.get());
         projects_main_targets_[i->source_node_->products_owner().get_project()].insert(i->source_node_->products_owner().get_main_target());
         
         if (projects_.find(i->source_node_->products_owner().get_project()) == projects_.end())
            projects_.insert(i->source_node_->products_owner().get_project(), 
                             std::auto_ptr<eclipse_cdt_project>(new eclipse_cdt_project(owner_, *i->source_node_->products_owner().get_project())));
      
         fill_projects(*i->source_node_, visited_nodes);
      }
   }

   for(build_nodes_t::const_iterator i = node.dependencies_.begin(), last = node.dependencies_.end(); i != last; ++i)
   {
      if (visited_nodes.find(i->get()) == visited_nodes.end())
      {
         visited_nodes.insert(i->get());
         projects_main_targets_[(**i).products_owner().get_project()].insert((**i).products_owner().get_main_target());

         if (projects_.find((**i).products_owner().get_project()) == projects_.end())
            projects_.insert((**i).products_owner().get_project(), 
                             std::auto_ptr<eclipse_cdt_project>(new eclipse_cdt_project(owner_, *(**i).products_owner().get_project())));

         fill_projects(**i, visited_nodes);
      }
   }
}

}}