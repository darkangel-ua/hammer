#include "stdafx.h"
#include <iostream>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <boost/format.hpp>
#include <hammer/core/generator_registry.h>
#include <hammer/core/project_requirements_decl.h>
#include "wildcard.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/toolset_manager.h>
#include <hammer/core/scaner_manager.h>
#include <hammer/core/default_output_location_strategy.h>
#include <hammer/core/warehouse_impl.h>
#include "builtin_features.h"

// v2
#include <hammer/parser/parser.h>
#include <hammer/sema/actions_impl.h>
#include <hammer/core/diagnostic.h>
#include <hammer/ast/context.h>
#include <hammer/core/rule_manager.h>
#include <hammer/core/ast2objects.h>
#include "builtin_rules.h"

using namespace std;
namespace fs = boost::filesystem;

namespace hammer{

engine::engine()
   :  feature_registry_(0),
      rule_manager_(new rule_manager)

{
   auto_ptr<hammer::feature_registry> fr(new hammer::feature_registry());

   details::install_builtin_rules(*rule_manager_);

   {
      feature_attributes ft = {0}; ft.free = 1;
      fr->add_feature_def("__searched_lib_name", vector<string>(), ft);
   }

   {
      // used to mark targets that belong to pch meta target. Needed for distinguishing PCH and OBJ generators
      feature_attributes ft = {0}; ft.free = 1;
      fr->add_feature_def("__pch", vector<string>(), ft);
   }

   {
      feature_attributes ft = {0}; ft.free = ft.generated = ft.path = 1;
      fr->add_feature_def("__generated-include", vector<string>(), ft);
   }

   {
      feature_attributes ft = {0};
      ft.propagated = true;
      fr->add_feature_def("host-os", {"windows", "linux"}, ft);
   }

#if defined(_WIN32)
   fr->get_def("host-os").set_default("windows");
#else
   fr->get_def("host-os").set_default("linux");
#endif

   feature_registry_ = fr.release();

   load_hammer_script(g_builtin_features, "builtin_features");

   switch(sizeof(nullptr)) {
      case 4:
         feature_registry_->get_def("address-model").set_default("32");
         break;
      case 8:
         feature_registry_->get_def("address-model").set_default("64");
         break;
   }

   type_registry_.reset(new type_registry);

   generators_.reset(new generator_registry);

   toolset_manager_.reset(new hammer::toolset_manager);
   scanner_manager_.reset(new hammer::scanner_manager);
   output_location_strategy_.reset(new default_output_location_strategy);
}

project* engine::get_upper_project(const location_t& project_path)
{
   // FIXME: BUG: boost parent_path() on "foo/bar/.  produce "foo/bar" instead of "foo"
   location_t upper_path = project_path.filename() == "." ? project_path.parent_path().parent_path() : project_path.parent_path();
   // FIXME: BUG: boost parent_path() can produce "E:" path and than "E:" / "foo" give as "E:foo" which is wrong
   if (upper_path.empty() || upper_path == project_path.root_name())
      return NULL;

   if (exists(upper_path / "hamfile"))
      return &load_project(upper_path);

   if (exists(upper_path / "hamroot"))
      return &load_project(upper_path);

   if (upper_path.has_parent_path())
      return get_upper_project(upper_path);
   else
      return NULL;
}

static location_t make_location(location_t::const_iterator first, location_t::const_iterator last)
{
   location_t result;
   for(; first != last; ++first)
      result /= *first;

   return result;
}

void engine::resolve_project_alias(resolved_project_symlinks_t& symlinks,
                                   location_t::const_iterator first, location_t::const_iterator last,
                                   global_project_links_t& symlink_storage)
{
   if (first == last)
      return;

   global_project_links_t::iterator i = symlink_storage.find(*first);
   if (i != symlink_storage.end())
   {
      ++first; // eat head and leave tail
      resolve_project_alias(symlinks, first, last, i->second->project_symlinks_);
      // We walk to see most longer alias and returns alias data for founded.
      // Only from one node.
      if (symlinks.empty() && !i->second->aliases_data_.empty())
         symlinks.push_back(resolved_project_symlink_t(make_location(first, last), i->second->aliases_data_));
   }
}

void engine::resolve_project_alias(resolved_project_symlinks_t& symlinks,
                                   const location_t& project_symlink)
{
   // ���������� ��������� ����
   resolve_project_alias(symlinks, ++project_symlink.begin(), project_symlink.end(), global_project_links_);
}

engine::loaded_projects_t
engine::load_project(location_t project_path,
                     const project& from_project)
{
   try {
      loaded_projects_t result(try_load_project(project_path, from_project));
      if (result.empty())
         throw std::runtime_error((boost::format("%s(0): error: can't load project '%s'")
                                      % from_project.location().string()
                                      % project_path).str());
      return result;
   } catch(const std::exception& e) {
      // FIXME: this is wrong approach, but I need to know where was a problem
      cerr << "While loading project '" << project_path << "'\n";
      throw;
   }
}

engine::loaded_projects_t
engine::try_load_project(location_t project_path,
                         const project& from_project)
{
   if (project_path.has_root_path()) {
      while(true) {
         loaded_projects_t result;
         resolved_project_symlinks_t symlinks;
         resolve_project_alias(symlinks, project_path);
         for(resolved_project_symlinks_t::const_iterator i = symlinks.begin(), last = symlinks.end(); i != last; ++i)
            for(project_alias_node::aliases_data_t::const_iterator j = i->symlinks_data_->begin(), j_last = i->symlinks_data_->end(); j != j_last; ++j)
               result += try_load_project(i->tail_, *j);

         if (warehouse_ && warehouse_->has_project(project_path, string())) {
            // If we can find project in warehouse than we need to add alternatives to resulting set of projects
            const project* materialized_warehouse_project = nullptr;
            for (const project* p : result) {
               if (warehouse_->project_from_warehouse(*p)) {
                  materialized_warehouse_project = p;
                  break;
               }
            }

            if (!materialized_warehouse_project) {
               boost::shared_ptr<project> not_yet_materialized_versions = warehouse_->load_project(*this, project_path);
               // we need to check if we already inserted this project before. When we doing warehouse::download_and_install we check
               // if project know to engine and that trigger loading project again, but engine may already manadge this project loaded before
               auto i = projects_.find(not_yet_materialized_versions->location());
               if (i == projects_.end()) {
                  // FIXME: Maybe we shouldn't add warehouse projects into engine, and manages them by warehouse
                  projects_.insert({not_yet_materialized_versions->location(), not_yet_materialized_versions});
                  result.push_back(not_yet_materialized_versions.get());
               } else
                  result.push_back(i->second.get());
            }
         } else
            return result;

         if (!result.empty())
            return result;
      }
   } else {
      location_t resolved_use_path, tail_path;
      resolve_use_project(resolved_use_path, tail_path,
                          from_project, project_path);
      location_t next_load_path(from_project.location() / resolved_use_path);
      next_load_path.normalize();
      if (tail_path.empty())
         return try_load_project(next_load_path);
      else
         return try_load_project(tail_path, load_project(next_load_path));
   }
}

engine::loaded_projects_t
engine::try_load_project(const location_t& tail_path,
                         const project_alias_data& symlink)
{
   project& p = load_project(symlink.location_);

   if (!tail_path.empty())
      return try_load_project(tail_path, p);
   else
      return loaded_projects_t(&p);
}

void engine::resolve_use_project(location_t& resolved_use_path, location_t& tail_path,
                                 const hammer::project& p, const location_t& path_to_resolve)
{
   use_project_data_t::const_iterator i = use_project_data_.find(&p);
   if (i == use_project_data_.end())
   {
      resolved_use_path = path_to_resolve;
      return;
   }

   use_project_data_t::mapped_type::const_iterator largets_use_iter = i->second.find(*path_to_resolve.begin());
   if (largets_use_iter == i->second.end())
   {
      resolved_use_path = path_to_resolve;
      return;
   }
   location_t larget_use_path = *path_to_resolve.begin();
   location_t::const_iterator to_resolve_begin = ++path_to_resolve.begin(),
                              to_resolve_end = path_to_resolve.end();

   for(;to_resolve_begin != to_resolve_end; ++to_resolve_begin)
   {
      location_t to_resolve = larget_use_path / *to_resolve_begin;
      use_project_data_t::mapped_type::const_iterator next_resolve_iter = i->second.find(to_resolve);
      if (next_resolve_iter == i->second.end())
         break;

      larget_use_path = to_resolve;
      largets_use_iter = next_resolve_iter;
   }

   resolved_use_path = largets_use_iter->second;

   // FIXME: stupid boost::filesystem::path can't be constructed from two iterators
   for(;to_resolve_begin != to_resolve_end; ++to_resolve_begin)
      tail_path /= *to_resolve_begin;
}

project& engine::load_project(location_t project_path)
{
   loaded_projects_t result(try_load_project(project_path));
   if (result.empty())
      throw  runtime_error("Can't load project at '"  + project_path.string() + ": no such path.");

   return result.front();
}

void engine::load_hammer_script(location_t filepath)
{
   filepath.normalize();
   if (!exists(filepath))
      return throw std::runtime_error("Hammer script '" + filepath.string() + "' doesn't exists.");

   projects_t::iterator i = projects_.find(filepath);
   if (i != projects_.end())
      throw std::runtime_error("Hammer script '" + filepath.string() + "' already loaded.");

   load_hammer_script_v2(filepath);
}

void engine::load_hammer_script(const string& script_body,
                                const string& script_name)
{
   projects_t::iterator i = projects_.find(script_name);
   if (i != projects_.end())
      throw std::runtime_error("Hammer script '" + script_name + "' already loaded.");

   load_hammer_script_v2(script_body, script_name);
}

void engine::load_hammer_script_v2(location_t filepath)
{
   ostringstream s;
   streamed_diagnostic diag(filepath.native(), s);
   ast::context ast_ctx;
   sema::actions_impl actions(ast_ctx, *rule_manager_, diag);
   ast_hamfile_ptr ast = parse_hammer_script(filepath, actions);

   if (diag.error_count())
      throw std::runtime_error("Parse errors: " + s.str());

   std::unique_ptr<project> loaded_project(new project(this));
   loaded_project->location(filepath.branch_path());
   invocation_context invc_ctx = { *loaded_project, diag, *rule_manager_ };

   try {
      ast2objects(invc_ctx, *ast);
   } catch (const ast2objects_semantic_error&) {
      throw std::runtime_error(s.str());
   }

   insert(loaded_project.get());
   loaded_project.release();
}

void engine::load_hammer_script_v2(const std::string& script_body,
                                   const std::string& script_name)
{
   ostringstream s;
   streamed_diagnostic diag(script_name, s);
   ast::context ast_ctx;
   sema::actions_impl actions(ast_ctx, *rule_manager_, diag);
   ast_hamfile_ptr ast = parse_hammer_script(script_body, script_name, actions);

   if (diag.error_count())
      throw std::runtime_error("Parse errors: " + s.str());

   std::unique_ptr<project> loaded_project(new project(this));
   loaded_project->location(script_name);
   invocation_context invc_ctx = { *loaded_project, diag, *rule_manager_ };

   try {
      ast2objects(invc_ctx, *ast);
   } catch (const ast2objects_semantic_error&) {
      throw std::runtime_error(s.str());
   }

   insert(loaded_project.get());
   loaded_project.release();
}

std::unique_ptr<project>
engine::load_project_v2(const location_t& project_path,
                        const project* upper_project)
{
   ostringstream s;
   streamed_diagnostic diag(project_path.native(), s);
   ast::context ast_ctx;
   sema::actions_impl actions(ast_ctx, *rule_manager_, diag);
   ast_hamfile_ptr ast = parse_hammer_script(project_path, actions);

   if (diag.error_count())
      throw std::runtime_error("Parse errors: " + s.str());

   std::unique_ptr<project> loaded_project(new project(this));
   loaded_project->location(project_path.branch_path());
   if (upper_project) {
      loaded_project->requirements().insert_infront(upper_project->requirements());
      loaded_project->usage_requirements().insert_infront(upper_project->usage_requirements());
   }

   invocation_context invc_ctx = { *loaded_project, diag, *rule_manager_ };

   try {
      ast2objects(invc_ctx, *ast);
   } catch (const ast2objects_semantic_error&) {
      throw std::runtime_error(s.str());
   }

   return loaded_project;
}

engine::loaded_projects_t engine::try_load_project(location_t project_path)
{
   location_t path_with_dot(project_path);
   path_with_dot /= ".";
   path_with_dot.normalize();

   projects_t::iterator i = projects_.find(path_with_dot);
   if (i != projects_.end())
      return loaded_projects_t(i->second.get());

   project_path.normalize();
   project* upper_project = NULL;

   if (!exists(project_path)) {
      upper_project = get_upper_project(project_path);
      if (upper_project == NULL)
         return loaded_projects_t();
   }

   location_t project_file = project_path / "hamfile";
   bool is_top_level = false;
   if (!exists(project_file)) {
      project_file = project_path / "hamroot";
      is_top_level = true;
   }
   else if (upper_project == NULL)
      upper_project = get_upper_project(project_path);

   if (!exists(project_file))
      return loaded_projects_t();

   std::unique_ptr<project> loaded_project = load_project_v2(project_file, is_top_level ? nullptr : upper_project);
   if (is_top_level)
      loaded_project->set_root(true);

   insert(loaded_project.get());
   project* p = loaded_project.release();

   return loaded_projects_t(p);
}

void engine::insert(project* p)
{
   projects_.insert(make_pair(p->location(), boost::shared_ptr<project>(p)));
}

engine::~engine()
{
   delete feature_registry_;
}

boost::filesystem::path find_root(const boost::filesystem::path& initial_path)
{
   boost::filesystem::path p(initial_path);

   while(true)
   {
      if (p.empty())
         throw runtime_error("Can't find boost-build.jam");

      if (exists(p / "boost-build.jam"))
         return p;

      p = p.branch_path();
   };
}

void engine::add_project_alias(project* p,
                               const std::string& project_id_alias,
                               const location_t& project_location,
                               feature_set* props)
{
   location_t l(project_id_alias);
   if (!l.has_root_path())
   {
      use_project_data_t::iterator i = use_project_data_.find(p);
      if (i != use_project_data_.end())
      {
         use_project_data_t::mapped_type::const_iterator j = i->second.find(project_location);
         if (j != i->second.end())
            throw std::runtime_error("alias '" + project_id_alias + "' already mapped to location '" + j->second + "'.");
      }

      use_project_data_[p].insert(make_pair(location_t(l), project_location.string()));
   }
   else
   {
      // go thought global map and create nodes that doesn't exists
      global_project_links_t* project_links_node = &global_project_links_;
      global_project_links_t::iterator alias_data_home;
      for(location_t::const_iterator i = ++l.begin(), last = l.end(); i != last; ++i)
      {
         alias_data_home = project_links_node->find(*i);
         if (alias_data_home == project_links_node->end())
         {
            boost::shared_ptr<project_alias_node> node(new project_alias_node);
            // FIXME stupid bug in ptr_map::insert
            location_t tmp(*i);
            alias_data_home = project_links_node->insert(make_pair(tmp, node)).first;
            project_links_node = &node->project_symlinks_;
         }
         else
            project_links_node = &alias_data_home->second->project_symlinks_;
      }

      project_alias_data alias_data;
      alias_data.location_ = p->location() / project_location;
      alias_data.location_.normalize();
      alias_data.properties_ = props;

      alias_data_home->second->aliases_data_.push_back(alias_data);

      reversed_global_project_links_.insert(make_pair(alias_data.location_, alias_data));
   }
}

void engine::setup_warehouse(const std::string& name,
                             const std::string& url,
                             const location_t& storage_dir)
{
   if (warehouse_)
      throw std::runtime_error("You can setup only one warehouse to use");

   warehouse_.reset(new warehouse_impl(name, url, storage_dir));
   load_project(storage_dir);
}

void engine::use_project(const project& p,
                         const std::string& project_id_alias,
                         const location_t& project_location)
{
   add_project_alias(const_cast<project*>(&p), project_id_alias, project_location, feature_registry().make_set());
}

project::selected_targets_t
engine::loaded_projects_t::select_best_alternative(const feature_set& build_request) const
{
   project::selected_targets_t result;
   for(projects_t::const_iterator i = projects_.begin(), last = projects_.end(); i != last; ++i)
   {
      project::selected_targets_t targets((**i).select_best_alternative(build_request));
      for(project::selected_targets_t::const_iterator t = targets.begin(), t_last = targets.end(); t != t_last; ++t)
         if (!t->target_->is_explicit())
            result.push_back(*t);
   }

   if (result.empty()) {
      stringstream s;
      dump_for_hash(s, build_request);
      throw std::runtime_error("Can't select best alternative - no one founded\n"
                               "Build request: " + s.str());
   }

   // Check for targets with same name. They already have same symbolic names so we should check names.
   sort(result.begin(), result.end(), [](const project::selected_target& lhs, const project::selected_target& rhs) {
      return lhs.target_->name() < rhs.target_->name();
   });

   auto first = result.begin();
   auto second = ++result.begin();
   for(; second != result.end();) {
      if (first->target_->name() == second->target_->name()) {
         stringstream s;
         dump_for_hash(s, build_request);
         throw std::runtime_error("Can't select best alternative for target '"+ first->target_->name() + "' from projects:\n"
                                  "1) '" + first->target_->location().string() + "' \n"
                                  "2) '" + second->target_->location().string() + "' \n"
                                  "Build request: " + s.str());
      } else {
         ++first;
         ++second;
      }
   }

   return result;
}

project::selected_target
engine::loaded_projects_t::select_best_alternative(const std::string& target_name,
                                                   const feature_set& build_request) const
{
   project::selected_targets_t result;
   for(projects_t::const_iterator i = projects_.begin(), last = projects_.end(); i != last; ++i)
   {
      project::selected_target st = (**i).try_select_best_alternative(target_name, build_request);
      if (st.target_ != NULL)
         result.push_back(st);
   }

   if (result.empty()) {
      stringstream s;
      s << "Can't select best alternative for target '"+ target_name + "' - no one founded. \n"
           "Projects to search are:\n";
      for (const project* p : projects_)
         s << "'" << p->location().string() << "'\n";
      s << "Build request: ";
      dump_for_hash(s, build_request);

      throw std::runtime_error(s.str());
   }

   if (result.size() == 1)
      return result.front();

   sort(result.begin(), result.end(), [](const project::selected_target& lhs, const project::selected_target& rhs) {
      return lhs.resolved_build_request_rank_ > rhs.resolved_build_request_rank_;
   });

   if (result[0].resolved_build_request_rank_ != result[1].resolved_build_request_rank_)
      return result.front();
   else {
      stringstream s;
      dump_for_hash(s, build_request);
      throw std::runtime_error("Can't select best alternative for target '"+ result[0].target_->name() + "' from projects:\n"
                               "1) '" + result[0].target_->location().string() + "' \n"
                               "2) '" + result[1].target_->location().string() + "' \n"
                               "Build request: " + s.str());
   }
}

void engine::output_location_strategy(boost::shared_ptr<hammer::output_location_strategy>& strategy)
{
   if (!strategy)
      output_location_strategy_.reset(new default_output_location_strategy);
   else
      output_location_strategy_ = strategy;
}

}
