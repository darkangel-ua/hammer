#include "builtin_rules.h"
#include <hammer/core/rule_manager.h>
#include <hammer/core/sources_decl.h>
#include <hammer/core/requirements_decl.h>
#include <hammer/core/typed_meta_target.h>
#include <hammer/core/types.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/alias_meta_target.h>
#include <hammer/core/lib_meta_target.h>
#include <hammer/core/header_lib_meta_target.h>
#include <hammer/core/prebuilt_lib_meta_target.h>
#include <hammer/core/version_alias_meta_target.h>
#include <hammer/core/target_version_alias_meta_target.h>
#include <hammer/core/copy_meta_target.h>
#include <hammer/core/obj_meta_target.h>
#include <hammer/core/testing_compile_fail_meta_target.h>
#include <hammer/core/testing_intermediate_meta_target.h>
#include <hammer/core/testing_meta_target.h>
#include <hammer/ast/target_ref.h>
#include <hammer/ast/path.h>
#include <hammer/ast/list_of.h>
#include <hammer/ast/casts.h>
#include <hammer/core/diagnostic.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/ast2objects.h>
#include <hammer/core/toolset_manager.h>
#include <boost/variant/get.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/make_unique.hpp>
#include "wildcard.hpp"

using namespace std;
namespace fs = boost::filesystem;

namespace hammer { namespace details {

static
void project_rule(invocation_context& ctx,
                  const parscore::identifier& id,
                  requirements_decl* requirements,
                  usage_requirements_decl* usage_requirements)
{
   ctx.current_project_.name(id.to_string());

   // we use insert because we already put there requirements/usage requirements from upper project
   if (requirements) {
      requirements->setup_path_data(&ctx.current_project_);
      ctx.current_project_.requirements().insert(*requirements);
   }

   if (usage_requirements) {
      usage_requirements->setup_path_data(&ctx.current_project_);
      ctx.current_project_.usage_requirements().insert(static_cast<const requirements_decl&>(*usage_requirements));
   }
}

static
void exe_rule(target_invocation_context& ctx,
              const parscore::identifier& id,
              const sources_decl& sources,
              const requirements_decl* requirements,
              const feature_set* default_build,
              const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new typed_meta_target(&ctx.current_project_,
                                                        id.to_string(),
                                                        requirements ? *requirements : requirements_decl(),
                                                        usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl(),
                                                        ctx.current_project_.get_engine()->get_type_registry().get(types::EXE)));
   mt->sources(sources);
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(mt);
}

static
void alias_rule(target_invocation_context& ctx,
                const parscore::identifier& name,
                const sources_decl& sources,
                const requirements_decl* requirements,
                const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new alias_meta_target(&ctx.current_project_,
                                                        name.to_string(),
                                                        sources,
                                                        requirements ? *requirements : requirements_decl(),
                                                        usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(mt);
}

static
void lib_rule(target_invocation_context& ctx,
              const parscore::identifier& id,
              const sources_decl& sources,
              const requirements_decl* requirements,
              const feature_set* default_build,
              const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new lib_meta_target(&ctx.current_project_,
                                                      id.to_string(),
                                                      requirements ? *requirements : requirements_decl(),
                                                      usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   mt->sources(sources);
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(mt);
}

static
void header_lib_rule(target_invocation_context& ctx,
                     const parscore::identifier& id,
                     const sources_decl* sources,
                     const requirements_decl* requirements,
                     const feature_set* default_build,
                     const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new header_lib_meta_target(&ctx.current_project_,
                                                             id.to_string(),
                                                             requirements ? *requirements : requirements_decl(),
                                                             usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   if (sources)
      mt->sources(*sources);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(mt);
}

static
void version_alias_rule(target_invocation_context& ctx,
                        const parscore::identifier& name,
                        const parscore::identifier& version,
                        const location_t* target_path)
{
   auto_ptr<basic_meta_target> mt(new version_alias_meta_target(&ctx.current_project_,
                                                                name.to_string(),
                                                                version.to_string(),
                                                                target_path ? &target_path->string() : nullptr));
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(mt);
}

static
void target_version_alias_rule(target_invocation_context& ctx,
                               const parscore::identifier& id,
                               const parscore::identifier& version,
                               const location_t* target_path)
{
   auto_ptr<basic_meta_target> mt(new target_version_alias_meta_target(&ctx.current_project_, id.to_string(), version.to_string(), target_path ? &target_path->string() : nullptr));

   mt->set_local(ctx.local_);
   // its explicit by design

   ctx.current_project_.add_target(mt);
}


static
bool has_dots(const location_t& l) {
   for (auto e : l)
      if (e == "..")
         return true;

   return false;
}

static
void use_project_rule(invocation_context& ctx,
                      const ast::expression& alias,
                      const location_t& location,
                      feature_set* props)
{
   std::string project_alias;

   if (const ast::id_expr* id = ast::as<ast::id_expr>(&alias)) {
      project_alias = id->id().to_string();
   } else if (const ast::path* p = ast::as<ast::path>(&alias)) {
      std::string s_path = p->to_string();
      if (p->root().valid() ||
          s_path.find_first_of("?*") != std::string::npos ||
          has_dots(location))
      {
         // FIXME: How to inform user about correct location?
         ctx.diag_.error({}, "Argument 'location': Expected non-root path without wildcards and ..");
         throw std::runtime_error("Sematic error");
      }
   } else if (const ast::target_ref* tr = ast::as<ast::target_ref>(&alias)) {
      if (tr->is_public() || !tr->build_request().empty() || tr->target_name().valid()) {
         ctx.diag_.error(tr->start_loc(), "Argument 'alias': Expected non-public target reference expression without target name and build request");
         throw std::runtime_error("Sematic error");
      }

      if (tr->target_path()->root().valid() && ctx.current_project_.is_root()) {
         ctx.diag_.error(alias.start_loc(), "Argument 'alias': Global aliases can be declared only in homroot file");
         throw std::runtime_error("Sematic error");
      }

      project_alias = tr->target_path()->to_string();
   } else {
      ctx.diag_.error(alias.start_loc(), "Argument 'alias': Expected target or id or path expression");
      throw std::runtime_error("Sematic error");
   }

   ctx.current_project_.get_engine()->add_project_alias(&ctx.current_project_, project_alias, location, props);
}

static
feature_attributes
resolve_attributes(const std::vector<std::string>& attributes)
{
   typedef std::vector<std::string>::const_iterator iter;
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

   i = find(attributes.begin(), attributes.end(), "no-checks");
   if (i != attributes.end())
      result.no_checks = true;

   i = find(attributes.begin(), attributes.end(), "generated");
   if (i != attributes.end())
      result.generated = true;

   return result;
}

static
std::vector<std::string>
ast2list_of_ids(invocation_context& ctx,
                const ast::expression* e)
{
   std::vector<std::string> result;
   if (!e)
      return result;

   else if (const ast::id_expr* id = ast::as<ast::id_expr>(e)) {
      result.push_back(id->id().to_string());
   } else if (const ast::list_of* list = ast::as<ast::list_of>(e)) {
      for (auto le : list->values()) {
         if (const ast::id_expr* id = ast::as<ast::id_expr>(le))
            result.push_back(id->id().to_string());
         else {
            ctx.diag_.error(le->start_loc(), "Expected id or list of ids expression");
            throw std::runtime_error("Sematic error");
         }
      }
   } else {
      ctx.diag_.error(e->start_loc(), "Expected id or list of ids expression");
      throw std::runtime_error("Sematic error");
   }

   return result;
}

static
void feature_feature_rule(invocation_context& ctx,
                          const parscore::identifier& name,
                          const ast::expression* ast_values,
                          const ast::expression* ast_attributes)
{
   std::vector<std::string> values = ast2list_of_ids(ctx, ast_values);
   std::vector<std::string> attributes = ast2list_of_ids(ctx, ast_attributes);
   ctx.current_project_.get_engine()->feature_registry().add_feature_def(name.to_string(), values, resolve_attributes(attributes));
}

static
void feature_local_rule(invocation_context& ctx,
                        const parscore::identifier& name,
                        const ast::expression* ast_values,
                        const ast::expression* ast_attributes)
{
   std::vector<std::string> values = ast2list_of_ids(ctx, ast_values);
   std::vector<std::string> attributes = ast2list_of_ids(ctx, ast_attributes);
   ctx.current_project_.local_feature_registry().add_feature_def(name.to_string(), values, resolve_attributes(attributes));
}

static
void feature_compose_rule(invocation_context& ctx,
                          const feature& f,
                          const feature_or_feature_set_t& components)
{
   feature_registry& fr = ctx.current_project_.get_engine()->feature_registry();
   feature_set* cc;
   if (auto f = boost::get<const feature*>(&components)) {
      cc = fr.make_set();
      cc->join(fr.clone_feature(**f));
   } else
      cc = (**boost::get<const feature_set*>(&components)).clone();

   fr.get_def(f.definition().name()).compose(f.value(), cc);
}

static
void feature_subfeature_rule(invocation_context& ctx,
                             const parscore::identifier& feature_name,
                             const parscore::identifier& subfeature_name)
{
   feature_def& def = ctx.current_project_.get_engine()->feature_registry().get_def(feature_name.to_string());
   def.add_subfeature(subfeature_name.to_string());
}

static
void variant_rule(invocation_context& ctx,
                  const parscore::identifier& variant_name,
                  const parscore::identifier* base,
                  feature_set& components)
{
   engine& e = *ctx.current_project_.get_engine();
   feature_def& def = e.feature_registry().get_def("variant");
   def.extend_legal_values(variant_name.to_string());

   if (base == NULL)
      def.compose(variant_name.to_string(), &components);
   else {
      feature_set* composite_features = e.feature_registry().make_set();
      def.expand_composites(base->to_string(), composite_features);
      composite_features->join(components);
      def.compose(variant_name.to_string(), composite_features);
   }
}

static
void glob_impl(sources_decl& result,
               const fs::path& searching_path,
               const fs::path& relative_path,
               const boost::dos_wildcard& wildcard,
               const std::vector<std::string>* exceptions,
               const type_registry& tr)
{
   for (fs::directory_iterator i(searching_path), last = fs::directory_iterator(); i != last; ++i) {
      if (!is_directory(*i) && wildcard.match(i->path().filename()) &&
          !(exceptions != 0 && find(exceptions->begin(), exceptions->end(), i->path().filename().string()) != exceptions->end()))
      {
         result.push_back((relative_path / i->path().filename()).string(), tr);
      }
   }
}

static
void rglob_impl(sources_decl& result,
                const fs::path& searching_path,
                fs::path relative_path,
                const boost::dos_wildcard& wildcard,
                const std::vector<std::string>* exceptions,
                const type_registry& tr)
{
   int level = 0;
   for (fs::recursive_directory_iterator i(searching_path), last = fs::recursive_directory_iterator(); i != last; ++i) {
      while(level > i.level()) {
         --level;
         relative_path = relative_path.branch_path();
      }

      if (is_directory(i.status())) {
         relative_path /= i->path().filename();
         ++level;
      } else if (wildcard.match(i->path().filename()) &&
                 !(exceptions != 0 && find(exceptions->begin(), exceptions->end(), i->path().filename().string()) != exceptions->end()))
      {
         result.push_back((relative_path / i->path().filename()).string(), tr);
      }
   }
}

static
std::unique_ptr<sources_decl>
glob_rule_impl(invocation_context& ctx,
               const path_or_list_of_paths_t& patterns,
               const path_or_list_of_paths_t* exceptions,
               const bool recursive)
{
   auto result = boost::make_unique<sources_decl>();
   std::vector<std::string> s_exceptions;
   if (exceptions) {
      if (auto p = boost::get<location_t>(exceptions))
         s_exceptions.push_back(p->string());
      else {
         const std::vector<location_t>& paths = *boost::get<std::vector<location_t>>(exceptions);
         transform(paths.begin(), paths.end(), back_inserter(s_exceptions), [](const location_t& l) { return l.string(); });
      }
   }

   std::vector<location_t> s_patterns;
   if (auto p = boost::get<location_t>(&patterns))
      s_patterns.push_back(p->string());
   else {
      const std::vector<location_t>& paths = boost::get<std::vector<location_t>>(patterns);
      transform(paths.begin(), paths.end(), back_inserter(s_patterns), [](const location_t& l) { return l.string(); });
   }

   for (auto& l_pattern : s_patterns ) {
      const string pattern = l_pattern.string();
      string::size_type mask_pos = pattern.find_first_of("*?");
      if (mask_pos == string::npos)
         throw runtime_error("[glob] You must specify patterns to match");
      string::size_type separator_pos = pattern.find_last_of("/\\", mask_pos);
      fs::path relative_path(separator_pos == string::npos ? fs::path() : fs::path(pattern.begin(),
                                                                          pattern.begin() + separator_pos));
      fs::path searching_path(ctx.current_project_.location() / relative_path);
      boost::dos_wildcard wildcard(string(pattern.begin() + mask_pos, pattern.end()));
      if (recursive)
         rglob_impl(*result, searching_path, relative_path, wildcard, &s_exceptions, ctx.current_project_.get_engine()->get_type_registry());
      else
         glob_impl(*result, searching_path, relative_path, wildcard, &s_exceptions, ctx.current_project_.get_engine()->get_type_registry());
   }

   result->unique();
   return result;
}

static
std::unique_ptr<sources_decl>
glob_rule(invocation_context& ctx,
          const path_or_list_of_paths_t& patterns,
          const path_or_list_of_paths_t* exceptions)
{
   return glob_rule_impl(ctx, patterns, exceptions, false);
}

static
std::unique_ptr<sources_decl>
rglob_rule(invocation_context& ctx,
           const path_or_list_of_paths_t& patterns,
           const path_or_list_of_paths_t* exceptions)
{
   return glob_rule_impl(ctx, patterns, exceptions, true);
}

static
void copy_rule(target_invocation_context& ctx,
               const parscore::identifier& name,
               const sources_decl& sources,
               const requirements_decl* requirements,
               const feature_set* default_build,
               const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new copy_meta_target(&ctx.current_project_,
                                                       name.to_string(),
                                                       requirements ? *requirements : requirements_decl(),
                                                       usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));

   mt->sources(sources);
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(mt);
}

static
void obj_rule(target_invocation_context& ctx,
              const parscore::identifier& name,
              const sources_decl& sources,
              const requirements_decl* requirements,
              const feature_set* default_build,
              const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new obj_meta_target(&ctx.current_project_,
                                                      name.to_string(),
                                                      requirements ? *requirements : requirements_decl(),
                                                      usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   mt->sources(sources);
   mt->set_local(ctx.local_);
   // its explicit by design
   ctx.current_project_.add_target(mt);
}

static
void test_suite_rule(target_invocation_context& ctx,
                     const parscore::identifier& name,
                     sources_decl& sources,
                     const sources_decl* propagated_sources)
{
   sources_decl empty_source;
   const sources_decl& real_propagated_sources = (propagated_sources == NULL ? empty_source : *propagated_sources);
   feature_set* additional_sources_set = ctx.current_project_.get_engine()->feature_registry().make_set();
   for(sources_decl::const_iterator i = real_propagated_sources.begin(), last = real_propagated_sources.end(); i != last; ++i) {
      feature* f = ctx.current_project_.get_engine()->feature_registry().create_feature("testing.additional-source", "");
      f->set_dependency_data(*i, &ctx.current_project_);
      additional_sources_set->join(f);
   }

   for(auto s : sources)
      if (s.properties() != NULL)
         s.properties()->join(*additional_sources_set);
      else
         s.properties(additional_sources_set);

   auto_ptr<basic_meta_target> mt(new alias_meta_target(&ctx.current_project_,
                                                        name.to_string(),
                                                        sources,
                                                        requirements_decl(),
                                                        requirements_decl()));

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(mt);
}

static
std::unique_ptr<sources_decl>
testing_run_rule(target_invocation_context& ctx,
                 const sources_decl* sources,
                 const std::vector<parscore::identifier>* args,
                 const std::vector<parscore::identifier>* input_files,
                 const requirements_decl* requirements,
                 const parscore::identifier* target_name)
{
   feature_registry& fr = ctx.current_project_.get_engine()->feature_registry();
   type_registry& tr = ctx.current_project_.get_engine()->get_type_registry();
   string real_target_name;
   if (target_name != NULL)
      real_target_name = target_name->to_string();
   else
      if (sources != NULL && !sources->empty())
         real_target_name = location_t(sources->begin()->target_path()).stem().string();
      else
         throw std::runtime_error("Target must have either sources or target name");

   const string& exe_name = real_target_name;
   auto_ptr<basic_meta_target> intermediate_exe(
      new testing_intermediate_meta_target(&ctx.current_project_,
                                           exe_name,
                                           requirements != NULL ? *requirements
                                                                : requirements_decl(),
                                           requirements_decl(),
                                           tr.get(types::EXE)));

   intermediate_exe->sources(sources == NULL ? sources_decl() : *sources);
   intermediate_exe->set_local(true);
   intermediate_exe->set_explicit(true);

   ctx.current_project_.add_target(intermediate_exe);

   requirements_decl run_requirements;

   if (input_files != NULL)
      for (auto f : *input_files)
         run_requirements.add(*fr.create_feature("testing.input-file", f.to_string()));

   if (args != NULL)
      for(auto a : *args)
         run_requirements.add(*fr.create_feature("testing.argument", a.to_string()));

   auto_ptr<basic_meta_target> run_target(
      new testing_meta_target(&ctx.current_project_,
                              real_target_name + ".runner",
                              run_requirements,
                              requirements_decl(),
                              tr.get(types::TESTING_RUN_PASSED)));

   sources_decl run_sources;
   run_sources.push_back(exe_name, tr);
   run_target->sources(run_sources);

   source_decl run_target_source(run_target->name(),
                                 std::string(),
                                 NULL /*to signal that this is meta target*/,
                                 NULL);

   ctx.current_project_.add_target(run_target);

   auto result = boost::make_unique<sources_decl>();
   result->push_back(run_target_source);

   return result;
}

static
std::unique_ptr<sources_decl>
testing_compile_fail_rule(target_invocation_context& ctx,
                          const sources_decl& sources,
                          const requirements_decl* requirements,
                          const requirements_decl* default_build,
                          const usage_requirements_decl* usage_requirements)
{
   const string target_name = location_t(sources.begin()->target_path()).stem().string();
   auto_ptr<basic_meta_target> mt(new testing_compile_fail_meta_target(&ctx.current_project_,
                                                                       target_name,
                                                                       requirements ? *requirements : requirements_decl(),
                                                                       usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   mt->sources(sources);

   const source_decl compile_source(mt->name(),
                                    std::string(),
                                    NULL /*to signal that this is meta target*/,
                                    NULL);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(mt);

   auto result = boost::make_unique<sources_decl>();
   result->push_back(compile_source);

   return result;
}

static
void prebuilt_lib_rule(target_invocation_context& ctx,
                       const parscore::identifier& name,
                       const sources_decl* sources,
                       const location_t& filename,
                       const requirements_decl* requirements,
                       const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new prebuilt_lib_meta_target(&ctx.current_project_,
                                                               name.to_string(),
                                                               filename.string(),
                                                               requirements ? *requirements : requirements_decl(),
                                                               usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   if (sources)
      mt->sources(*sources);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(mt);
}

static
void searched_shared_lib_rule(target_invocation_context& ctx,
                              const parscore::identifier& name,
                              const sources_decl* sources,
                              const parscore::identifier& libname,
                              const requirements_decl* requirements,
                              const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new searched_lib_meta_target(&ctx.current_project_,
                                                               name.to_string(),
                                                               libname.to_string(),
                                                               requirements ? *requirements : requirements_decl(),
                                                               usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl(),
                                                               ctx.current_project_.get_engine()->get_type_registry().get(types::SEARCHED_SHARED_LIB)));
   if (sources)
      mt->sources(*sources);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(mt);
}

static
void searched_static_lib_rule(target_invocation_context& ctx,
                              const parscore::identifier& name,
                              const sources_decl* sources,
                              const parscore::identifier& libname,
                              const requirements_decl* requirements,
                              const usage_requirements_decl* usage_requirements)
{
   auto_ptr<basic_meta_target> mt(new searched_lib_meta_target(&ctx.current_project_,
                                                               name.to_string(),
                                                               libname.to_string(),
                                                               requirements ? *requirements : requirements_decl(),
                                                               usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl(),
                                                               ctx.current_project_.get_engine()->get_type_registry().get(types::SEARCHED_STATIC_LIB)));
   if (sources)
      mt->sources(*sources);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(mt);
}

static
void use_toolset_rule(invocation_context& ctx,
                      const parscore::identifier& toolset_name,
                      const parscore::identifier& toolset_version,
                      const location_t* toolset_home_)
{
   location_t toolset_home;
   if (toolset_home_ != NULL)
      toolset_home = *toolset_home_;

   engine& e = *ctx.current_project_.get_engine();
   e.toolset_manager().init_toolset(e, toolset_name.to_string(), toolset_version.to_string(), toolset_home_ == NULL ? NULL : &toolset_home);
}

static
void setup_warehouse_rule(invocation_context& ctx,
                          const parscore::identifier& name,
                          const parscore::identifier& url,
                          const location_t* storage_dir_)
{
   string storage_dir;
   if (storage_dir_ && !storage_dir_->empty()) {
      fs::path sd(*storage_dir_);
      if (sd.is_absolute())
         storage_dir = sd.string();
      else {
         sd = ctx.current_project_.location() / sd;
         sd.normalize();
         storage_dir = sd.string();
      }
   }

   ctx.current_project_.get_engine()->setup_warehouse(name.to_string(), url.to_string(), storage_dir);
}

void install_builtin_rules(rule_manager& rm)
{
   rm.add_rule("project", project_rule, {"id", "requirements", "usage-requirements"});
   rm.add_rule("use-project", use_project_rule, {"alias", "location", "requirements"});
   rm.add_rule("feature.feature", feature_feature_rule, {"name", "values", "attributes"});
   rm.add_rule("feature.local", feature_local_rule, {"name", "values", "attributes"});
   rm.add_rule("feature.compose", feature_compose_rule, {"feature", "components"});
   rm.add_rule("feature.subfeature", feature_subfeature_rule, {"feature-name", "subfeature-name"});
   rm.add_rule("variant", variant_rule, {"name", "base", "components"});
   rm.add_rule("glob", glob_rule, {"patterns", "exceptions"});
   rm.add_rule("rglob", rglob_rule, {"patterns", "exceptions"});
   rm.add_target("exe", exe_rule, {"name", "sources", "requirements", "default-build", "usage-requirements"});
   rm.add_target("lib", lib_rule, {"name", "sources", "requirements", "default-build", "usage-requirements"});
   rm.add_target("alias", alias_rule, {"name", "sources", "requirements", "usage-requirements"});
   rm.add_target("version-alias", version_alias_rule, {"name", "version", "target-path"});
   rm.add_target("target-version-alias", target_version_alias_rule, {"name", "version", "target-path"});
   rm.add_target("header-lib", header_lib_rule, {"name", "sources", "requirements", "default-build", "usage-requirements"});
   rm.add_target("prebuilt-lib", prebuilt_lib_rule, {"name", "sources", "filename", "requirements", "usage-requirements"});
   rm.add_target("searched-shared-lib", searched_shared_lib_rule, {"name", "sources", "libname", "requirements", "usage-requirements"});
   rm.add_target("searched-static-lib", searched_static_lib_rule, {"name", "sources", "libname", "requirements", "usage-requirements"});
   rm.add_target("copy", copy_rule, {"name", "sources", "requirements", "default-build", "usage-requirements"});
   rm.add_target("obj", obj_rule, {"name", "sources", "requirements", "default-build", "usage-requirements"});
   rm.add_target("test-suite", test_suite_rule, {"name", "sources", "propagated-sources"});
   rm.add_target("testing.run", testing_run_rule, {"sources", "args", "input-files", "requirements", "target-name"});
   rm.add_target("testing.compile-fail", testing_compile_fail_rule, {"sources", "requirements", "default-build", "usage-requirements"});
   rm.add_rule("use-toolset", use_toolset_rule, {"name", "version", "home"});
   rm.add_rule("setup-warehouse", setup_warehouse_rule, {"name", "url", "storage-dir"});
}

}}
