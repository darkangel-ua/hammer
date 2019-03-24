#include "stdafx.h"
#include <iostream>
#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/project_requirements_decl.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/toolset_manager.h>
#include <hammer/core/scaner_manager.h>
#include <hammer/core/default_output_location_strategy.h>
#include <hammer/core/warehouse_impl.h>
#include <hammer/parser/parser.h>
#include <hammer/sema/actions_impl.h>
#include <hammer/core/diagnostic.h>
#include <hammer/ast/context.h>
#include <hammer/core/rule_manager.h>
#include <hammer/core/ast2objects.h>
#include <hammer/core/warehouse_manager.h>
#include "wildcard.hpp"
#include "builtin_rules.h"
#include "builtin_features.h"

using namespace std;
namespace fs = boost::filesystem;

static const string hamfile = "hamfile";
static const string hamroot = "hamroot";

namespace hammer {

engine::engine()
   :  global_project_(new project(*this, nullptr, {})),
      feature_registry_(new hammer::feature_registry),
      rule_manager_(new rule_manager),
      warehouse_manager_(new hammer::warehouse_manager)
{
   details::install_builtin_rules(*rule_manager_);
   load_hammer_script(g_builtin_features, "builtin_features");

   {
      feature_attributes ft = {0}; ft.free = 1;
      feature_registry_->add_feature_def("__searched_lib_name", {}, ft);
   }

   {
      // used to mark targets that belong to pch meta target. Needed for distinguishing PCH and OBJ generators
      feature_attributes ft = {0}; ft.free = 1;
      feature_registry_->add_feature_def("__pch", {}, ft);
   }

   {
      feature_attributes ft = {0}; ft.free = ft.generated = ft.path = 1;
      feature_registry_->add_feature_def("__generated-include", {}, ft);
   }

   {
      feature_attributes ft = {0};
      ft.propagated = true;
      feature_registry_->add_feature_def("host-os", { {"windows", {}}, {"linux", {}} }, ft);
   }

   switch(sizeof(nullptr)) {
      case 4:
         feature_registry_->get_def("address-model").set_default("32");
         break;
      case 8:
         feature_registry_->get_def("address-model").set_default("64");
         break;
   }

#if defined(_WIN32)
   feature_registry_->get_def("host-os").set_default("windows");
   feature_registry_->get_def("target-os").set_default("windows");
#else
   feature_registry_->get_def("host-os").set_default("linux");
   feature_registry_->get_def("target-os").set_default("linux");
#endif

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
      return nullptr;

   if (exists(upper_path / hamfile))
      return &load_project(upper_path);

   if (exists(upper_path / hamroot))
      return &load_project(upper_path);

   if (upper_path.has_parent_path())
      return get_upper_project(upper_path);
   else
      return nullptr;
}

loaded_projects
engine::load_project(const global_project_ref& project_ref)
{
   assert(project_ref.value_.has_root_directory());
   return global_project_->load_project(project_ref.value_.relative_path());
}

project& engine::load_project(location_t fs_project_path)
{
   loaded_projects result{try_load_project(fs_project_path)};
   if (result.empty())
      throw  runtime_error("Can't load project at '"  + fs_project_path.string() + "': no such path.");

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

   load_hammer_script_v2(nullptr, filepath);
}

void engine::load_hammer_script(const string& script_body,
                                const string& script_name)
{
   projects_t::iterator i = projects_.find(script_name);
   if (i != projects_.end())
      throw std::runtime_error("Hammer script '" + script_name + "' already loaded.");

   load_hammer_script_v2(nullptr, script_body, script_name);
}

static
void load_hammer_script_impl(engine& e,
                             const project* parent,
                             ostringstream& s,
                             diagnostic& diag,
                             const location_t& project_location,
                             const ast::hamfile& ast)
{
   if (diag.error_count())
      throw parsing_error(s.str());

   std::unique_ptr<project> loaded_project(new project(e, parent, project_location));
   invocation_context invc_ctx = { *loaded_project, diag, e.get_rule_manager() };

   try {
      ast2objects(invc_ctx, ast);
   } catch (const ast2objects_semantic_error&) {
      throw parsing_error(s.str());
   }

   e.insert(move(loaded_project));
}

struct parser_environment : sema::actions_impl::environment {
   parser_environment(const feature_registry& fr) : fr_(fr) {}
   bool known_feature(const parscore::identifier& name) const override {
      return fr_.find_def(name.to_string());
   }

   const feature_registry& fr_;
};

void engine::load_hammer_script_v2(const project* parent,
                                   location_t filepath)
{
   ostringstream s;
   streamed_diagnostic diag(filepath.string(), error_verbosity_, s);
   ast::context ast_ctx;
   parser_environment env(*feature_registry_);
   sema::actions_impl actions(ast_ctx, env, *rule_manager_, diag);
   ast_hamfile_ptr ast = parse_hammer_script(filepath, actions);

   load_hammer_script_impl(*this, parent, s, diag, filepath.branch_path(), *ast);
}

void engine::load_hammer_script_v2(const project* parent,
                                   const std::string& script_body,
                                   const std::string& script_name)
{
   ostringstream s;
   streamed_diagnostic diag(script_name, error_verbosity_, s);
   ast::context ast_ctx;
   parser_environment env(*feature_registry_);
   sema::actions_impl actions(ast_ctx, env, *rule_manager_, diag);
   ast_hamfile_ptr ast = parse_hammer_script(script_body, script_name, actions);

   load_hammer_script_impl(*this, parent, s, diag, script_name, *ast);
}

std::unique_ptr<project>
engine::load_project_v2(const location_t& project_path,
                        const project* upper_project)
{
   ostringstream s;
   streamed_diagnostic diag(project_path.string(), error_verbosity_, s);
   ast::context ast_ctx;
   parser_environment env(*feature_registry_);
   sema::actions_impl actions(ast_ctx, env, *rule_manager_, diag);
   ast_hamfile_ptr ast = parse_hammer_script(project_path, actions);

   if (diag.error_count())
      throw parsing_error(s.str());

   std::unique_ptr<project> loaded_project(new project(*this, upper_project, project_path.branch_path()));

   invocation_context invc_ctx = { *loaded_project, diag, *rule_manager_ };

   try {
      ast2objects(invc_ctx, *ast);
   } catch (const ast2objects_semantic_error&) {
      throw parsing_error(s.str());
   }

   return loaded_project;
}

loaded_projects
engine::try_load_project(location_t fs_project_path)
{
   location_t path_with_dot(fs_project_path);
   path_with_dot /= ".";
   path_with_dot.normalize();

   projects_t::iterator i = projects_.find(path_with_dot);
   if (i != projects_.end())
      return loaded_projects{i->second.get()};

   fs_project_path.normalize();
   project* upper_project = nullptr;

   if (!exists(fs_project_path)) {
      upper_project = get_upper_project(fs_project_path);
      if (!upper_project)
         return {};
   }

   location_t project_file = fs_project_path / hamfile;
   bool is_top_level = false;
   if (!exists(project_file)) {
      project_file = fs_project_path / hamroot;
      is_top_level = true;
   }
   else if (!upper_project)
      upper_project = get_upper_project(fs_project_path);

   if (!exists(project_file))
      return {};

   std::unique_ptr<project> loaded_project = load_project_v2(project_file, is_top_level ? nullptr : upper_project);
   if (is_top_level)
      loaded_project->set_root(true);

   return loaded_projects{&insert(move(loaded_project))};
}

project&
engine::insert(std::unique_ptr<project> p)
{
   projects_.insert({p->location(), boost::shared_ptr<project>(p.get())});
   return *p.release();
}

engine::~engine()
{
}

bool has_project_file(const boost::filesystem::path& folder) {
   return exists(folder / hamfile) || exists(folder / hamroot);
}

void engine::add_alias(const location_t& alias_path,
                       const location_t& full_project_path,
                       feature_set* props,
                       const project::alias::match match_strategy)
{
   assert(full_project_path.has_root_path());
   assert(alias_path.has_root_directory());
   global_project_->add_alias(alias_path.relative_path(), full_project_path, props, match_strategy);
}

void engine::output_location_strategy(boost::shared_ptr<hammer::output_location_strategy>& strategy)
{
   if (!strategy)
      output_location_strategy_.reset(new default_output_location_strategy);
   else
      output_location_strategy_ = strategy;
}

template<typename Callback>
void map_over_aliases(engine& e,
                      const project::aliases_t& aliases,
                      const std::string& prefix,
                      Callback&& cb) {
   for (auto& alias : aliases) {
      if (alias.is_transparent())
         map_over_aliases(e, e.load_project(alias.full_fs_path_).aliases(), prefix, cb);
      else {
         auto& p = e.load_project(alias.full_fs_path_);
         auto new_prefix = prefix + "/" + alias.alias_.string();

         cb(new_prefix, p);

         map_over_aliases(e, p.aliases(), new_prefix, cb);
      }
   }
}

boost::regex
target_ref_mask_to_regex(const std::string& mask) {
   string regexyfied_mask{mask};

   // use regex where * has been replaced on #
   // last step is to replace back # by *
   boost::replace_all(regexyfied_mask, "**", ".#?");
   boost::replace_all(regexyfied_mask, "*", "[^/]#");
   boost::replace_all(regexyfied_mask, "#", "*");

   return boost::regex{regexyfied_mask};
}

vector<reference_wrapper<const project>>
resolve_project_query(engine& e,
                      const std::string& query_) {
   if (query_.empty() || query_.front() != '/')
      throw std::runtime_error("Project query must me non empty and begins with /");

   string regexyfied_query{query_};

   // use regex where * has been replaced on #
   // last step is to replace back # by *
   boost::replace_all(regexyfied_query, "**", ".#?");
   boost::replace_all(regexyfied_query, "*", "[^/]#");
   boost::replace_all(regexyfied_query, "#", "*");

   const boost::regex rquery{regexyfied_query};
   vector<reference_wrapper<const project>> result;
   map_over_aliases(e, e.global_aliases(), {}, [&](const std::string& alias, const project& p) {
      if (boost::regex_match(alias, rquery) && p.publishable())
         result.push_back(std::cref(p));
   });

   return result;
}

boost::optional<boost::filesystem::path>
find_root(boost::filesystem::path from_path) {
   if (!from_path.has_root_path())
      throw std::runtime_error("[find_root]: from_path should be absolute");

   from_path.normalize();
   auto& p = from_path;

   while (!p.empty()) {
      if (exists(p / hamroot))
         return p;

      p = p.branch_path();
   };

   return {};
}

}

