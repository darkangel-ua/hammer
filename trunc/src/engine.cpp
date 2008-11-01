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
   resolver_.insert("feature.feature", boost::function<void (project*, vector<pstring>&, vector<pstring>*, vector<pstring>&)>(boost::bind(&engine::feature_feature_rule, this, _1, _2, _3, _4)));
   resolver_.insert("feature.compose", boost::function<void (project*, feature&, feature_set&)>(boost::bind(&engine::feature_compose_rule, this, _1, _2, _3)));
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
   add_msvc_generators(*this, generators());

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
      // ��������� ��� �������� � ������ ����� ����� ���� ��� ��������������� ��� ����������� 
      // ��������������� ��������� ��������
      feature_set::const_iterator scm_client_name_feature = alias_data.properties_->find("scm");
      if (scm_client_name_feature != alias_data.properties_->end())
         p.scm_info().scm_client_name_ = (**scm_client_name_feature).value();

      feature_set::const_iterator scm_url_feature = alias_data.properties_->find("scm.url");
      if (scm_url_feature != alias_data.properties_->end())
         p.scm_info().scm_url_ = (**scm_url_feature).value();
   }
}

bool engine::materialize_or_load_next_repository(project_alias_data& alias_data)
{
   if (repositories_.empty())
      return false;

   repository_data& rep_data = repositories_.front();
   if (repositories_.front().materialized_)
      return false;

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

location_t engine::resolve_project_alias(project_alias_data& alias_data, const location_t& loc)
{
   // ���������� ��������� ����
   location_t::const_iterator i = ++loc.begin(), last = loc.end();
   location_t searched_loc = location_t("/") / *i;
   
   global_project_links_t::const_iterator g;
   for(;;)
   {
      g = global_project_links_.find(searched_loc);
      if (g == global_project_links_.end())
      {
         ++i;
         if (i != last)
            searched_loc /= *i;
         else 
         {
            if (!materialize_or_load_next_repository(alias_data))
               throw runtime_error("Can't resolve global project link '" + loc.string() + "'.");
            else // Restart searching from the beginning
            {
               i = ++loc.begin();
               last = loc.end();
               searched_loc = location_t("/") / *i;
               continue;
            }
         }
      }
      else 
         break;
   }

   location_t result;
   for(++i; i != last; ++i)
      result /= *i;
   
   alias_data = g->second;

   return result;
}


void engine::initial_materialization(const project_alias_data& alias_data) const
{
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

project& engine::load_project(location_t project_path, const project& from_project)
{
   if (project_path.has_root_path())
   {
      project_alias_data alias_data;
      location_t tail_path = resolve_project_alias(alias_data, project_path);
      project_path = alias_data.location_;
      if (!exists(project_path))
      {
         const project* upper_materialized_project = find_upper_materialized_project(project_path);
         if (upper_materialized_project == NULL)
            initial_materialization(alias_data);
         else
            materialize_directory(resolve_scm_client(*upper_materialized_project), project_path, false);
      }

      project& p = load_project(project_path);
      update_project_scm_info(p, alias_data);
     
      if (!tail_path.empty())
         return load_project(tail_path, p);
      else
         return p;
   }
   else
   {
      location_t resolved_use_path, tail_path;
      resolve_use_project(resolved_use_path, tail_path, 
                          from_project, project_path);
      if (tail_path.empty())
         return load_project(from_project.location() / resolved_use_path);
      else
         return load_project(tail_path, load_project(from_project.location() / resolved_use_path));
   }
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

const scm_client& engine::resolve_scm_client(const project& p)
{
   const project* upper_materialized_project = find_upper_materialized_project(p);
   if (upper_materialized_project == NULL)
      throw std::runtime_error("Can't find scm client for project at location '" + p.location().string() + "'.");

   std::string scm_client_name = upper_materialized_project->scm_info().scm_client_name_.to_string();
   const hammer::scm_client* scm_client = scm_manager_->find(scm_client_name);
   if (!scm_client)
      throw runtime_error("SCM client '" + scm_client_name + "' not supported.");

   return *scm_client;
}

void engine::materialize_project(const location_t& project_path, 
                                 const project& upper_project)
{
   materialize_directory(resolve_scm_client(upper_project), project_path);
}

project& engine::load_project(location_t project_path)
{
   projects_t::iterator i = projects_.find(project_path);
   if (i != projects_.end())
      return *i->second;

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
            throw  runtime_error("Can't load project at '"  + project_path.string() + ": no such path.");

         materialize_project(project_path, *upper_project);
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

      if (!p.parse(project_file.native_file_string().c_str()))
         throw  runtime_error("Can't load project at '"  + project_path.string() + ": parser errors");

      // �� ���� ��� �� ������ ������� ��� ������ ����� ��� ����� ��� ��������� �����������
      // � ��������� �� ��� ����� �������
      if (!is_top_level)
      {
         if (upper_project)
         {
            ctx.project_->requirements().insert_infront(upper_project->requirements());
            ctx.project_->usage_requirements().insert_infront(upper_project->usage_requirements());
         }
      }

      p.walk(&ctx);
      assert(ctx.project_);
      insert(ctx.project_);
      return *ctx.project_;
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
   
   // ����� ������������� ������� ��� ��� �� ����� �������� 
   // engine ��� ��������� �������������� �������� ������� �� ��� ��������
   if (req)
      p->requirements().insert(req->requirements());
   if (usage_req)
      p->usage_requirements().insert(usage_req->requirements());
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

static feature_attributes resolve_attributes(std::vector<pstring>& attributes)
{
   typedef std::vector<pstring>::const_iterator iter;
   feature_attributes result = {0};
   
   iter i = find(attributes.begin(), attributes.end(), "propagated");
   if (i != attributes.end())
      result.propagated = true;

   i = find(attributes.begin(), attributes.end(), "composite");
   if (i != attributes.end())
      result.composite = true;

   i = find(attributes.begin(), attributes.end(), "free");
   if (i != attributes.end())
      result.free = true;

   i = find(attributes.begin(), attributes.end(), "path");
   if (i != attributes.end())
      result.path = true;
   
   i = find(attributes.begin(), attributes.end(), "incidental");
   if (i != attributes.end())
      result.incidental = true;

   i = find(attributes.begin(), attributes.end(), "optional");
   if (i != attributes.end())
      result.optional = true;

   i = find(attributes.begin(), attributes.end(), "symmetric");
   if (i != attributes.end())
      result.symmetric = true;

   i = find(attributes.begin(), attributes.end(), "dependency");
   if (i != attributes.end())
      result.dependency = true;

   i = find(attributes.begin(), attributes.end(), "no-defaults");
   if (i != attributes.end())
      result.no_defaults = true;

   return result;
}

void engine::feature_feature_rule(project* p, std::vector<pstring>& name, 
                                  std::vector<pstring>* values,
                                  std::vector<pstring>& attributes)
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
   feature_registry_->get_def(f.def().name()).compose(f.value().to_string(), cc);
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
      global_project_links_t::const_iterator i = global_project_links_.find(l);
      if (i != global_project_links_.end())
         throw runtime_error("Project id alias '" + project_id_alias.to_string() + 
                             "' already mapped to location '" + i->second.location_.string() + "'.");
      project_alias_data alias_data;
      alias_data.location_ = p->location() / project_location.to_string();
      alias_data.location_.normalize();
      alias_data.properties_ = props;

      global_project_links_.insert(make_pair(l, alias_data));
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

}
