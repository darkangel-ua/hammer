#include "stdafx.h"
#include "engine.h"
#include "hammer_walker_context.h"
#include "type_registry.h"
#include "types.h"
#include "basic_target.h"
#include "feature_registry.h"
#include "feature_set.h"
#include "feature.h"
#include "parser.h"
#include <boost/bind.hpp>
#include "lib_meta_target.h"
#include "typed_meta_target.h"
#include "alias_meta_target.h"
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include "generator_registry.h"
#include "msvc_generator.h"
#include "project_requirements_decl.h"
#include "wildcard.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/logic/tribool.hpp>
#include "scm_manager.h"
#include "fs_helpers.h"
#include "header_lib_meta_target.h"
#include "pch_meta_target.h"

using namespace std;
namespace fs = boost::filesystem;

namespace hammer{

engine::engine() 
   :  feature_registry_(0)
{
   type_registry_.reset(new type_registry);
   types::register_standart_types(*type_registry_);

   auto_ptr<hammer::feature_registry> fr(new hammer::feature_registry(&pstring_pool()));

   resolver_.insert("project", boost::function<void (project*, vector<pstring>&, project_requirements_decl*, project_requirements_decl*)>(boost::bind(&engine::project_rule, this, _1, _2, _3, _4)));
   resolver_.insert("lib", boost::function<void (project*, vector<pstring>&, sources_decl*, requirements_decl*, feature_set*, requirements_decl*)>(boost::bind(&engine::lib_rule, this, _1, _2, _3, _4, _5, _6)));
   resolver_.insert("header-lib", boost::function<void (project*, vector<pstring>&, sources_decl*, requirements_decl*, feature_set*, requirements_decl*)>(boost::bind(&engine::header_lib_rule, this, _1, _2, _3, _4, _5, _6)));
   resolver_.insert("exe", boost::function<void (project*, vector<pstring>&, sources_decl&, requirements_decl*, feature_set*, requirements_decl*)>(boost::bind(&engine::exe_rule, this, _1, _2, _3, _4, _5, _6)));
   resolver_.insert("obj", boost::function<void (project*, pstring&, sources_decl&, requirements_decl*, feature_set*, requirements_decl*)>(boost::bind(&engine::obj_rule, this, _1, _2, _3, _4, _5, _6)));
   resolver_.insert("pch", boost::function<void (project*, pstring&, sources_decl&, requirements_decl*, feature_set*, requirements_decl*)>(boost::bind(&engine::pch_rule, this, _1, _2, _3, _4, _5, _6)));
   resolver_.insert("alias", boost::function<void (project*, pstring&, sources_decl*, requirements_decl*, feature_set*, requirements_decl*)>(boost::bind(&engine::alias_rule, this, _1, _2, _3, _4, _5, _6)));
   resolver_.insert("import", boost::function<void (project*, vector<pstring>&)>(boost::bind(&engine::import_rule, this, _1, _2)));
   resolver_.insert("feature.feature", boost::function<void (project*, vector<pstring>&, vector<pstring>*, vector<pstring>*)>(boost::bind(&engine::feature_feature_rule, this, _1, _2, _3, _4)));
   resolver_.insert("feature.compose", boost::function<void (project*, feature&, feature_set&)>(boost::bind(&engine::feature_compose_rule, this, _1, _2, _3)));
   resolver_.insert("variant", boost::function<void (project*, pstring&, pstring*, feature_set&)>(boost::bind(&engine::variant_rule, this, _1, _2, _3, _4)));
   resolver_.insert("glob", boost::function<sources_decl (project*, std::vector<pstring>&, std::vector<pstring>*)>(boost::bind(&engine::glob_rule, this, _1, _2, _3, false)));
   resolver_.insert("rglob", boost::function<sources_decl (project*, std::vector<pstring>&, std::vector<pstring>*)>(boost::bind(&engine::glob_rule, this, _1, _2, _3, true)));
   resolver_.insert("explicit", boost::function<void (project*, const pstring&)>(boost::bind(&engine::explicit_rule, this, _1, _2)));
   resolver_.insert("use-project", boost::function<void (project*, const pstring&, const pstring&, feature_set*)>(boost::bind(&engine::use_project_rule, this, _1, _2, _3, _4)));
   resolver_.insert("repository", boost::function<void (project*, const pstring&, feature_set*)>(boost::bind(&engine::repository_rule, this, _1, _2, _3)));

   {
      feature_attributes ft = {0}; ft.free = 1;
      fr->add_def(feature_def("__searched_lib_name", vector<string>(), ft));
   }

   {
      feature_attributes ft = {0}; ft.generated = 1;
      fr->add_def(feature_def("__use_pch", boost::assign::list_of<string>("off")("on"), ft));
   }

   {
      feature_attributes ft = {0}; ft.generated = ft.free = 1;
      fr->add_def(feature_def("__create_pch", vector<string>(), ft));
   }

   {
      // used to mark targets that belong to pch meta target. Needed for distinguishing PCH and OBJ generators
      feature_attributes ft = {0}; ft.free = 1;
      fr->add_def(feature_def("__pch", vector<string>(), ft));
   }

   feature_registry_ = fr.release();

   generators_.reset(new generator_registry);

   scm_manager_.reset(new scm_manager);
}

project* engine::get_upper_project(const location_t& project_path)
{
   location_t upper_path = project_path.branch_path();
   if (exists(upper_path / "hamfile"))
      return &load_project(upper_path);
   
   if (exists(upper_path / "hamroot"))
      return &load_project(upper_path);
   
   if (upper_path.has_leaf())
      return get_upper_project(upper_path);
   else
      return 0;
}

void engine::update_project_scm_info(project& p, const project_alias_data& alias_data) const
{
   if (p.scm_info().scm_url_.empty() && 
       alias_data.properties_ != NULL) // alias_data.properties_ can be null if no features was specified in use-project rule
   {
      // добавляем эти свойства в проект чтобы можна было ими воспользоваться при последующих 
      // материализациях вложенных проектов
      feature_set::const_iterator scm_client_name_feature = alias_data.properties_->find("scm");
      if (scm_client_name_feature != alias_data.properties_->end())
         p.scm_info().scm_client_name_ = (**scm_client_name_feature).value();

      feature_set::const_iterator scm_url_feature = alias_data.properties_->find("scm.url");
      if (scm_url_feature != alias_data.properties_->end())
         p.scm_info().scm_url_ = (**scm_url_feature).value();
   }
}

bool engine::materialize_or_load_next_repository()
{
   if (repositories_.empty())
      return false;

   repository_data& rep_data = repositories_.front();
   if (repositories_.front().materialized_)
      return false;
   
   project_alias_data alias_data;
   location_t repository_full_location(rep_data.defined_in_project_->location() / rep_data.location_);
   alias_data.location_ = repository_full_location;
   alias_data.properties_ = rep_data.properties_;
   if (!exists(repository_full_location))
      initial_materialization(alias_data);

   project& loaded_project = load_project(repository_full_location);
   update_project_scm_info(loaded_project, alias_data);

   // rotating left repositories
   rep_data.materialized_ = true;
   repositories_.push_back(rep_data);
   repositories_.pop_front();
   
   return true;
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
      if (!i->second->aliases_data_.empty())
         symlinks.push_back(resolved_project_symlink_t(make_location(first, last), i->second->aliases_data_));
      resolve_project_alias(symlinks, first, last, i->second->project_symlinks_);
   }
}

void engine::resolve_project_alias(resolved_project_symlinks_t& symlinks,
                                   const location_t& project_symlink)
{
   // пропускаем начальный слеш
   resolve_project_alias(symlinks, ++project_symlink.begin(), project_symlink.end(), global_project_links_);
}

void engine::initial_materialization(const project_alias_data& alias_data) const
{
   if (alias_data.properties_ == NULL)
      throw std::runtime_error("Fail to materialize project at '" + alias_data.location_.native_file_string() + "'");

   feature_set::const_iterator scm_tag = alias_data.properties_->find("scm");
   if (scm_tag == alias_data.properties_->end())
      throw runtime_error("Can't materialize project because no scm feature specified.");
   
   feature_set::const_iterator scm_uri = alias_data.properties_->find("scm.url");
   if (scm_uri == alias_data.properties_->end())
      throw runtime_error("Can't materialize project because no scm url specified.");

   const hammer::scm_client* scm_client = scm_manager_->find((**scm_tag).value().to_string());
   if (!scm_client)
      throw runtime_error("SCM client '" + (**scm_tag).value().to_string() + "' not supported.");
   
   boost::filesystem::create_directories(alias_data.location_);
   scm_client->checkout(alias_data.location_, (**scm_uri).value().to_string(), false);
}

static void materialize_directory(const scm_client& scm_client, location_t dir, bool recursive = true)
{
   string what_up;
   while(!exists(dir)) 
   {
      what_up = dir.leaf();
      dir = dir.branch_path();
   }

   scm_client.up(dir, what_up, recursive);
}

engine::loaded_projects_t 
engine::load_project(location_t project_path, 
                     const project& from_project)
{
   loaded_projects_t result(try_load_project(project_path, from_project));
   if (result.empty())
      throw std::runtime_error((boost::format("%s(0): error: can't load project '%s'") 
                                   % from_project.location().native_file_string() 
                                   % project_path).str());

   return result;
}

engine::loaded_projects_t 
engine::try_load_project(location_t project_path, 
                         const project& from_project)
{
   if (project_path.has_root_path())
   {
      while(true)
      {
         loaded_projects_t result;
         resolved_project_symlinks_t symlinks;
         resolve_project_alias(symlinks, project_path);
         for(resolved_project_symlinks_t::const_iterator i = symlinks.begin(), last = symlinks.end(); i != last; ++i)
            for(project_alias_node::aliases_data_t::const_iterator j = i->symlinks_data_->begin(), j_last = i->symlinks_data_->end(); j != j_last; ++j)
               result += try_load_project(i->tail_, *j);
         
         if (!result.empty())
            return result;

         if (!materialize_or_load_next_repository())
            return result;
      }
   }
   else
   {
      location_t resolved_use_path, tail_path;
      resolve_use_project(resolved_use_path, tail_path,
                          from_project, project_path);
      if (tail_path.empty())
         return try_load_project(from_project.location() / resolved_use_path);
      else
         return try_load_project(tail_path, load_project(from_project.location() / resolved_use_path));
   }
}

engine::loaded_projects_t
engine::try_load_project(const location_t& tail_path, 
                         const project_alias_data& symlink)
{
   location_t project_path = symlink.location_;
   if (!exists(project_path))
   {
      // if alias data exists than use it
      const project* upper_materialized_project = 
         symlink.properties_ == NULL ? find_upper_materialized_project(project_path) 
                                        : NULL;
      if (upper_materialized_project == NULL)
         initial_materialization(symlink);
      else
      {
         const scm_client* scm_client = try_resolve_scm_client(*upper_materialized_project);
         if (scm_client == NULL)
            return loaded_projects_t();

         materialize_directory(*scm_client, project_path, false);
      }
   }

   project& p = load_project(project_path);
   update_project_scm_info(p, symlink);
  
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
   
   use_project_data_t::mapped_type::const_iterator j = i->second.find(*path_to_resolve.begin());
   if (j == i->second.end())
   {
      resolved_use_path = path_to_resolve;
      return;
   }
   
   resolved_use_path = j->second;
   if (!exists(p.location() / resolved_use_path))
      materialize_directory(resolve_scm_client(p), p.location() / resolved_use_path, false);

   // FIXME: stupid boost::filesystem::path can't be constructed from two iterators
   for(location_t::const_iterator i = ++path_to_resolve.begin(), last = path_to_resolve.end(); i != last; ++i)
      tail_path /= *i;
}

const project* 
engine::find_upper_materialized_project(const project& p)
{
   if (!p.scm_info().scm_url_.empty())
      return &p;
   else
   {
      if (p.is_root())
         return NULL;

      const project* upper_project = get_upper_project(p.location());
      if (upper_project != NULL)
         return find_upper_materialized_project(*upper_project);
      else
         return NULL;
   }
}

const project* engine::find_upper_materialized_project(const location_t& location)
{
   project* p = get_upper_project(location);
   return p == NULL ? NULL : find_upper_materialized_project(*p);
}

const scm_client& engine::resolve_scm_client_impl(const project& p)
{
   std::string scm_client_name = p.scm_info().scm_client_name_.to_string();
   const hammer::scm_client* scm_client = scm_manager_->find(scm_client_name);
   if (!scm_client)
      throw runtime_error("SCM client '" + scm_client_name + "' not supported.");

   return *scm_client;
}

const scm_client* engine::try_resolve_scm_client(const project& p)
{
   const project* upper_materialized_project = find_upper_materialized_project(p);
   if (upper_materialized_project == NULL)
      return NULL;
   
   return &resolve_scm_client_impl(*upper_materialized_project);
}

const scm_client& engine::resolve_scm_client(const project& p)
{
   const project* upper_materialized_project = find_upper_materialized_project(p);
   if (upper_materialized_project == NULL)
         throw std::runtime_error("Can't find scm client for project at location '" + p.location().string() + "'.");

   return resolve_scm_client_impl(*upper_materialized_project);
}

bool engine::try_materialize_project(const location_t& project_path, 
                                     const project& upper_project)
{
   const scm_client* scm_client = try_resolve_scm_client(upper_project);
   if (scm_client == NULL)
      return false;
   
   materialize_directory(*scm_client, project_path);
   return true;
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
      return throw std::runtime_error("Hammer script '" + filepath.native_file_string() + "' doesn't exists.");

   projects_t::iterator i = projects_.find(filepath);
   if (i != projects_.end())
      throw std::runtime_error("Hammer script '" + filepath.native_file_string() + "' already loaded.");

   hammer_walker_context ctx;

   try
   {
      ctx.engine_ = this;
      ctx.location_ = filepath;
      ctx.project_ = new project(this);
      ctx.project_->location(filepath);
      ctx.call_resolver_ = &resolver_;

      parser p(this);
      if (!p.parse(filepath.native_file_string().c_str()))
         throw  runtime_error("Can't load script at '"  + filepath.string() + ": parser errors");

      p.walk(&ctx);
      assert(ctx.project_);
      insert(ctx.project_);
   }
   catch(...)
   {
      delete ctx.project_;
      throw;
   }
}

engine::loaded_projects_t engine::try_load_project(location_t project_path)
{
   projects_t::iterator i = projects_.find(project_path);
   if (i != projects_.end())
      return loaded_projects_t(i->second);

   hammer_walker_context ctx;
   try
   {
      project_path.normalize();
      ctx.engine_ = this;
      ctx.location_ = project_path;
      ctx.project_ = new project(this);
      ctx.project_->location(project_path);
      ctx.call_resolver_ = &resolver_;
      project* upper_project = NULL;

      parser p(this);
      if (!exists(project_path))
      {
         upper_project = get_upper_project(project_path);
         if (upper_project == NULL)
            return loaded_projects_t();

         if (!try_materialize_project(project_path, *upper_project))
            return loaded_projects_t();
      }

      location_t project_file = project_path / "hamfile";
      bool is_top_level = false;
      if (!exists(project_file))
      {
         project_file = project_path / "hamroot";
         is_top_level = true;
      }
      else 
         if (upper_project == NULL)
            upper_project = get_upper_project(project_path);

      if (!exists(project_file))
         return loaded_projects_t();

      if (!p.parse(project_file.native_file_string().c_str()))
         throw  runtime_error("Can't load project at '"  + project_path.string() + ": parser errors");

      // до того как мы начнем парсить сам проект нужно уже знать все реквизиты предыдущего
      // и выставить их для этого проекта
      if (!is_top_level)
      {
         if (upper_project)
         {
            ctx.project_->requirements().insert_infront(upper_project->requirements());
            ctx.project_->usage_requirements().insert_infront(upper_project->usage_requirements());
         }
      }
      else
         ctx.project_->set_root(true);

      p.walk(&ctx);
      assert(ctx.project_);
      insert(ctx.project_);
      return loaded_projects_t(ctx.project_);
   }
   catch(...)
   {
      delete ctx.project_;
      throw;
   }
}

void engine::insert(project* p)
{
   projects_.insert(p->location(), p);
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

void engine::project_rule(project* p, std::vector<pstring>& name,
                          project_requirements_decl* req, project_requirements_decl* usage_req)
{
   assert(name.size() == size_t(1));
   p->name(name[0]);
   
   // здесь производиться вставка так как на этаме загрузки 
   // engine уже выставила унаследованные свойства проекта от его родителя
   if (req)
   {
      req->requirements().setup_path_data(p);
      p->requirements().insert(req->requirements());
   }

   if (usage_req)
   {
      usage_req->requirements().setup_path_data(p);
      p->usage_requirements().insert(usage_req->requirements());
   }
}

void engine::lib_rule(project* p, std::vector<pstring>& name, sources_decl* sources, requirements_decl* requirements,
                      feature_set* default_build, requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new lib_meta_target(p, name.at(0), requirements ? *requirements : requirements_decl(), 
                                                      usage_requirements ? *usage_requirements : requirements_decl()));
   if (sources)
      mt->sources(*sources);

   p->add_target(mt);
}

void engine::header_lib_rule(project* p, std::vector<pstring>& name, sources_decl* sources, requirements_decl* requirements,
                             feature_set* default_build, requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new header_lib_meta_target(p, name.at(0), requirements ? *requirements : requirements_decl(), 
                                                             usage_requirements ? *usage_requirements : requirements_decl()));
   if (sources)
      mt->sources(*sources);

   p->add_target(mt);
}

void engine::exe_rule(project* p, std::vector<pstring>& name, sources_decl& sources, requirements_decl* requirements,
                      feature_set* default_build, requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new typed_meta_target(p, name.at(0), requirements ? *requirements : requirements_decl(), 
                                                        usage_requirements ? *usage_requirements : requirements_decl(), 
                                                        get_type_registry().get(types::EXE)));
   mt->sources(sources);
   p->add_target(mt);
}

void engine::obj_rule(project* p, pstring& name, sources_decl& sources, requirements_decl* requirements,
                      feature_set* default_build, requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new typed_meta_target(p, name, requirements ? *requirements : requirements_decl(), 
                                                        usage_requirements ? *usage_requirements : requirements_decl(), 
                                                        get_type_registry().get(types::OBJ)));
   mt->sources(sources);
   p->add_target(mt);
}

void engine::pch_rule(project* p, pstring& name, sources_decl& sources, requirements_decl* requirements,
                      feature_set* default_build, requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new pch_meta_target(p, name, requirements ? *requirements : requirements_decl(), 
      usage_requirements ? *usage_requirements : requirements_decl()));
   mt->sources(sources);
   p->add_target(mt);
}

void engine::import_rule(project* p, std::vector<pstring>& name)
{

}

static feature_attributes resolve_attributes(std::vector<pstring>* attributes)
{
   typedef std::vector<pstring>::const_iterator iter;
   feature_attributes result = {0};
   
   if (attributes == NULL)
      return result; 

   iter i = find(attributes->begin(), attributes->end(), "propagated");
   if (i != attributes->end())
      result.propagated = true;

   i = find(attributes->begin(), attributes->end(), "composite");
   if (i != attributes->end())
      result.composite = true;

   i = find(attributes->begin(), attributes->end(), "free");
   if (i != attributes->end())
      result.free = true;

   i = find(attributes->begin(), attributes->end(), "path");
   if (i != attributes->end())
      result.path = true;
   
   i = find(attributes->begin(), attributes->end(), "incidental");
   if (i != attributes->end())
      result.incidental = true;

   i = find(attributes->begin(), attributes->end(), "optional");
   if (i != attributes->end())
      result.optional = true;

   i = find(attributes->begin(), attributes->end(), "symmetric");
   if (i != attributes->end())
      result.symmetric = true;

   i = find(attributes->begin(), attributes->end(), "dependency");
   if (i != attributes->end())
      result.dependency = true;

   i = find(attributes->begin(), attributes->end(), "no-defaults");
   if (i != attributes->end())
      result.no_defaults = true;

   i = find(attributes->begin(), attributes->end(), "no-checks");
   if (i != attributes->end())
      result.no_checks = true;

   return result;
}

void engine::feature_feature_rule(project* p, std::vector<pstring>& name, 
                                  std::vector<pstring>* values,
                                  std::vector<pstring>* attributes)
{
   if (name.empty() || name.size() > 1)
      throw std::runtime_error("[feature.feature] Bad feature name.");

   vector<string> def_values;
   if (values)
   {
      for(vector<pstring>::const_iterator i = values->begin(), last = values->end(); i != last; ++i)
         def_values.push_back(i->to_string());
   }

   feature_def def(name[0].to_string(), def_values, resolve_attributes(attributes));

   feature_registry_->add_def(def);
}

void engine::feature_compose_rule(project* p, feature& f, feature_set& components)
{
   feature_set* cc = components.clone();
   feature_registry_->get_def(f.definition().name()).compose(f.value().to_string(), cc);
}

void engine::variant_rule(project* p, pstring& variant_name, pstring* base, feature_set& components)
{
   feature_def& def = feature_registry_->get_def("variant");
   def.extend_legal_values(variant_name.to_string());

   if (base == NULL)
      def.compose(variant_name.to_string(), &components);
   else
   {
      feature_set* composite_features = feature_registry_->make_set();
      def.expand_composites(base->to_string(), composite_features);
      composite_features->join(components);
      def.compose(variant_name.to_string(), composite_features);
   }
}

void engine::alias_rule(project* p, 
                        pstring& name, 
                        sources_decl* sources, 
                        requirements_decl* requirements, 
                        feature_set* default_build, 
                        requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new alias_meta_target(p, name, 
                                                        sources == NULL ? sources_decl() : *sources, 
                                                        requirements ? *requirements : requirements_decl(), 
                                                        usage_requirements ? *usage_requirements : requirements_decl()));
   p->add_target(mt);
}

static void glob_impl(sources_decl& result,
                      const fs::path& searching_path,
                      const fs::path& relative_path,
                      const boost::dos_wildcard& wildcard,             
                      const std::vector<pstring>* exceptions,
                      engine& e)
{
   for(fs::directory_iterator i(searching_path), last = fs::directory_iterator(); i != last; ++i)
   {
      if (!is_directory(*i) && wildcard.match(i->leaf()) && 
          !(exceptions != 0 && find(exceptions->begin(), exceptions->end(), i->leaf()) != exceptions->end()))
      {
         pstring v(e.pstring_pool(), (relative_path / i->leaf()).string());
         result.push_back(v, e.get_type_registry());
      }
   }
}

static void rglob_impl(sources_decl& result,
                       const fs::path& searching_path,
                       fs::path relative_path,
                       const boost::dos_wildcard& wildcard,             
                       const std::vector<pstring>* exceptions,
                       engine& e)
{
   int level = 0;
   for(fs::recursive_directory_iterator i(searching_path), last = fs::recursive_directory_iterator(); i != last; ++i)
   {
      while(level > i.level())
      {
         --level;
         relative_path = relative_path.branch_path();
      }

      if (is_directory(i.status()))
      {
         relative_path /= i->leaf();
         ++level;
      }
      else
         if (wildcard.match(i->leaf()) && 
             !(exceptions != 0 && 
               find(exceptions->begin(), exceptions->end(), i->leaf()) != exceptions->end()))
         {
            pstring v(e.pstring_pool(), (relative_path / i->leaf()).string());
            result.push_back(v, e.get_type_registry());
         }
   }
}

sources_decl engine::glob_rule(project* p, std::vector<pstring>& patterns, 
                               std::vector<pstring>* exceptions, bool recursive)
{
   using namespace boost::filesystem;
   sources_decl result;

   typedef std::vector<pstring>::const_iterator iter;
   for(iter i = patterns.begin(), last = patterns.end(); i != last; ++i)
   {
      string pattern(i->to_string());
      string::size_type mask_pos = pattern.find_first_of("*?");
      if (mask_pos == string::npos)
         throw runtime_error("[glob] You must specify patterns to match");
      string::size_type separator_pos = pattern.find_last_of("/\\", mask_pos);
      path relative_path(separator_pos == string::npos ? path() : path(pattern.begin(), 
                                                                       pattern.begin() + separator_pos));
      path searching_path(p->location() / relative_path);
      boost::dos_wildcard wildcard(string(pattern.begin() + mask_pos, pattern.end()));
      if (recursive)
         rglob_impl(result, searching_path, relative_path, wildcard, exceptions, *this);
      else
         glob_impl(result, searching_path, relative_path, wildcard, exceptions, *this);
   }
   
   result.unique();
   return result;
}

void engine::explicit_rule(project* p, const pstring& target_name)
{
   basic_meta_target* target = p->find_target(target_name);
   if (target == 0)
      throw std::runtime_error("target '" + target_name.to_string() + "' not found.");
   target->set_explicit(true);
}

void engine::use_project_rule(project* p, const pstring& project_id_alias, 
                              const pstring& project_location, feature_set* props)
{
   location_t l(project_id_alias.to_string());
   if (!l.has_root_path())
   {
      use_project_data_t::iterator i = use_project_data_.find(p);
      if (i != use_project_data_.end())
      {
         use_project_data_t::mapped_type::const_iterator j = i->second.find(project_location.to_string());
         if (j != i->second.end())
            throw std::runtime_error("alias '" + project_id_alias.to_string() + "' already mapped to location '" + j->second + "'.");
      }

      use_project_data_[p].insert(make_pair(location_t(l), project_location.to_string()));
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
            std::auto_ptr<project_alias_node> node(new project_alias_node);
            // FIXME stupid bug in ptr_map::insert
            location_t tmp(*i);
            alias_data_home = project_links_node->insert(tmp, node.get()).first;
            project_links_node = &node->project_symlinks_;
            node.release();
         }
         else
            project_links_node = &alias_data_home->second->project_symlinks_;
      }
      
      project_alias_data alias_data;
      alias_data.location_ = p->location() / project_location.to_string();
      alias_data.location_.normalize();
      alias_data.properties_ = props;

      alias_data_home->second->aliases_data_.push_back(alias_data);
   }
}

void engine::repository_rule(project* p, const pstring& a_project_location, feature_set* props)
{
   location_t project_location(a_project_location.to_string());
   repositories_t::const_iterator i = find_if(repositories_.begin(), repositories_.end(), 
                                              boost::bind(&repository_data::location_, _1) == boost::ref(project_location));
   if (i != repositories_.end())
      throw runtime_error((boost::format("Repository with placement at '%s' already defined.") % project_location.string()).str());

   repositories_.push_back(repository_data(p, project_location, props));
}

static bool targets_by_name(const basic_meta_target* lhs, const basic_meta_target* rhs)
{
   return lhs->name() < rhs->name();
}

static bool has_override(engine& e, 
                         const basic_meta_target& target, 
                         const feature_set& build_request)
{
   feature_set* fs = e.feature_registry().make_set();
   target.requirements().eval(build_request, fs);
   return fs->find("override") != fs->end();
}

void engine::loaded_projects_t::post_process(project::selected_targets_t& result,
                                             const feature_set& build_request) const
{
   if (result.empty())
      throw std::runtime_error("[FIXME] Can't select best alternative - no one founded");

   if (result.size() == 1)
      return;

   // Check for targets with same name. They already have same symbolic names so we should check names.
   using namespace boost::logic;
   engine& e = *result.front()->project()->engine();
   std::sort(result.begin(), result.end(), targets_by_name);
   project::selected_targets_t::iterator 
      first = result.begin(), 
      second = ++result.begin();
   tribool overriden = false;
   for(; second != result.end();)
   {
      if ((**first).name() == (**second).name())
      {
         switch(overriden.value)
         {
            case tribool::true_value:
            {
               if (has_override(e, **second, build_request))
                  throw std::runtime_error("[FIXME] Can't select best alternative from two others");
               else
                  second = result.erase(second);
               
               break;
            }

            case tribool::false_value:
            {
               if (has_override(e, **second, build_request))
               {
                  std::swap(*first, *second);
                  second = result.erase(second);
               }
               else
                  throw std::runtime_error("[FIXME] Can't select best alternative from two others");
               
               break;
            }

            case tribool::indeterminate_value:
            {
               overriden = has_override(e, **first, build_request);
               break;
            }
         }
      }
      else
      {
         ++first;
         ++second;
      }
   }
}

project::selected_targets_t 
engine::loaded_projects_t::select_best_alternative(const feature_set& build_request) const
{
   project::selected_targets_t result;
   for(projects_t::const_iterator i = projects_.begin(), last = projects_.end(); i != last; ++i)
   {
      project::selected_targets_t targets((**i).select_best_alternative(build_request));
      for(project::selected_targets_t::const_iterator t = targets.begin(), t_last = targets.end(); t != t_last; ++t)
         if (!(**t).is_explicit())
            result.push_back(*t);
   }

   post_process(result, build_request);

   return result;
}

const basic_meta_target* 
engine::loaded_projects_t::select_best_alternative(const pstring& target_name, 
                                                   const feature_set& build_request) const
{
   project::selected_targets_t result;
   for(projects_t::const_iterator i = projects_.begin(), last = projects_.end(); i != last; ++i)
   {
      const basic_meta_target* target = (**i).try_select_best_alternative(target_name, build_request);
      if (target != NULL)
         result.push_back(target);
   }

   // after this call we will have only one meta target in result list
   post_process(result, build_request);

   return result.front();
}

}
