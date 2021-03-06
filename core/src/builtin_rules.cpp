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
#include <hammer/core/copy_meta_target.h>
#include <hammer/core/obj_meta_target.h>
#include <hammer/core/testing_compile_fail_meta_target.h>
#include <hammer/core/testing_compile_meta_target.h>
#include <hammer/core/testing_link_fail_meta_target.h>
#include <hammer/core/testing_link_meta_target.h>
#include <hammer/core/testing_intermediate_meta_target.h>
#include <hammer/core/testing_run_meta_target.h>
#include <hammer/core/testing_suite_meta_target.h>
#include <hammer/core/target_type.h>
#include <hammer/ast/target_ref.h>
#include <hammer/ast/path.h>
#include <hammer/ast/list_of.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/requirement_set.h>
#include <hammer/core/diagnostic.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/ast2objects.h>
#include <hammer/core/toolset_manager.h>
#include <hammer/core/warehouse_manager.h>
#include <hammer/core/warehouse_impl.h>
#include <boost/make_unique.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/variant/get.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/make_unique.hpp>
#include <boost/regex.hpp>
#include <boost/unordered_set.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "builtin_rules.h"
#include "wildcard.hpp"

using namespace std;
using boost::make_unique;
namespace fs = boost::filesystem;
namespace hammer { namespace details {

static boost::regex project_id_pattern("[a-zA-Z0-9_.\\-]+");

static
const ast::expression*
project_id_validator(ast::context& ctx,
                     diagnostic& diag,
                     const ast::expression* e)
{
   if (!e)
      return e;

   if (const ast::id_expr* id = ast::as<ast::id_expr>(e)) {
      if (id->id().valid()) {
         const string& s_id = id->id().to_string();
         if (s_id == "." || s_id == ".." || !boost::regex_match(s_id, project_id_pattern)) {
            diag.error(e->start_loc(), "project id should not be '.' or '..' and should match '%s' regex") << project_id_pattern.str().c_str();
            return new (ctx) ast::error_expression(id);
         }
      } // otherwise its empty id
   } else if (const ast::path* id = ast::as<ast::path>(e)) {
      if (id->root_name().valid()) {
         diag.error(id->root_name().start_loc(), "project id cannot have root name");
         return new (ctx) ast::error_expression(id);
      }

      for (const ast::expression* pe : id->elements()) {
         if (const ast::id_expr* e_id = ast::as<ast::id_expr>(pe)) {
            const string& s_id = e_id->id().to_string();
            if (s_id == "." || s_id == ".." || !boost::regex_match(s_id, project_id_pattern)) {
               diag.error(e_id->start_loc(), "project id path element should not be '.' or '..' and should match '%s' regex") << project_id_pattern.str().c_str();
               return new (ctx) ast::error_expression(id);
            }
         } else {
            diag.error(pe->start_loc(), "project id path element should not contains wildcard elements");
            return new (ctx) ast::error_expression(id);
         }
      }
   } else {
      diag.error(e->start_loc(), "project id must be simple ID or path with elements that matches '%s' regex") << project_id_pattern.str().c_str();
      return new (ctx) ast::error_expression(e);
   }

   return e;
}

static
void process_project_id(invocation_context& ctx,
                        const ast::expression* e)
{
   if (!e)
      return;

   if (const ast::id_expr* id = ast::as<ast::id_expr>(e)) {
      if (id->id().valid())
         ctx.current_project_.name(id->id().to_string());
      // otherwise its empty id
   } else {
      const ast::path* p = ast::as<ast::path>(e);
      ctx.current_project_.name(p->to_string());
   }
}

static
const ast::expression*
default_build_checker(ast::context& ctx,
                      diagnostic& diag,
                      const ast::expression* e) {
   assert(ast::as<ast::requirement_set>(e));
   auto rs = ast::as<ast::requirement_set>(e);

   auto process_one = [&] (const ast::expression* requirement) -> const ast::expression* {
      if (ast::as<ast::feature>(requirement))
         return requirement;
      else {
         diag.error(e->start_loc(), "Only plain features allowed here");
         return new (ctx) ast::error_expression(requirement);
      }
   };

   if (const ast::list_of* l = ast::as<ast::list_of>(rs->requirements())) {
      auto elements = ast::expressions_t{ast::expressions_t::allocator_type{ctx}};
      for (const ast::expression* e : l->values())
         elements.push_back(process_one(e));
      return new (ctx) ast::requirement_set(new (ctx) ast::list_of(elements));
   } else
      return new (ctx) ast::requirement_set(process_one(rs->requirements()));
}

static
void project_rule(invocation_context& ctx,
                  const ast::expression* id,
                  requirements_decl* requirements,
                  usage_requirements_decl* usage_requirements,
                  const one_or_list<details::project_dependency>* dependencies,
                  const requirements_decl* default_build)
{
   process_project_id(ctx, id);

   if (requirements)
      ctx.current_project_.local_requirements(*requirements);

   if (usage_requirements)
      ctx.current_project_.local_usage_requirements(static_cast<const requirements_decl&>(*usage_requirements));

   if (dependencies) {
      project::dependencies_t deps;
      for (auto& d : *dependencies) {
         feature_set* props = ctx.current_project_.feature_registry().make_set();
         props->join("version", d.version_.to_string().c_str());
         deps.push_back({target_ref_mask_to_regex(d.pattern_.value_.string()), props});
      }
      ctx.current_project_.dependencies(std::move(deps));
   }

   if (default_build)
      ctx.current_project_.default_build(*default_build);
}

static
void exe_rule(target_invocation_context& ctx,
              const parscore::identifier& id,
              const sources_decl& sources,
              const requirements_decl* requirements,
              const requirements_decl* default_build,
              const usage_requirements_decl* usage_requirements)
{
   unique_ptr<basic_meta_target> mt(new typed_meta_target(&ctx.current_project_,
                                                          id.to_string(),
                                                          requirements ? *requirements : requirements_decl(),
                                                          usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl(),
                                                          ctx.current_project_.get_engine().get_type_registry().get(types::EXE)));
   mt->sources(sources);
   if (default_build)
      mt->default_build(*default_build);
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(move(mt));
}

static
void alias_rule(target_invocation_context& ctx,
                const parscore::identifier& name,
                const sources_decl* sources,
                const requirements_decl* requirements,
                const usage_requirements_decl* usage_requirements)
{
   unique_ptr<basic_meta_target> mt(new alias_meta_target(&ctx.current_project_,
                                                          name.to_string(),
                                                          sources ? *sources : sources_decl(),
                                                          requirements ? *requirements : requirements_decl(),
                                                          usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(move(mt));
}

static
void lib_rule(target_invocation_context& ctx,
              const parscore::identifier& id,
              const sources_decl& sources,
              const requirements_decl* requirements,
              const requirements_decl* default_build,
              const usage_requirements_decl* usage_requirements)
{
   unique_ptr<basic_meta_target> mt(new lib_meta_target(&ctx.current_project_,
                                                        id.to_string(),
                                                        requirements ? *requirements : requirements_decl(),
                                                        usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   mt->sources(sources);
   if (default_build)
      mt->default_build(*default_build);
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(move(mt));
}

static
void header_lib_rule(target_invocation_context& ctx,
                     const parscore::identifier& id,
                     const sources_decl* sources,
                     const requirements_decl* requirements,
                     const requirements_decl* default_build,
                     const usage_requirements_decl* usage_requirements)
{
   unique_ptr<basic_meta_target> mt(new header_lib_meta_target(&ctx.current_project_,
                                                               id.to_string(),
                                                               sources ? *sources : sources_decl{},
                                                               requirements ? *requirements : requirements_decl(),
                                                               usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   if (sources)
      mt->sources(*sources);
   if (default_build)
      mt->default_build(*default_build);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);

   ctx.current_project_.add_target(move(mt));
}

static
const ast::expression*
use_project_alias_validator(ast::context& ctx,
                            diagnostic& diag,
                            const ast::expression* alias)
{
   if (!alias)
      return alias;

   if (ast::as<ast::id_expr>(alias)) {
      return alias;
   } else if (const ast::path* p = ast::as<ast::path>(alias)) {
      if (p->root_name().valid() || p->has_wildcard()) {
         diag.error(alias->start_loc(), "Argument 'alias': Expected non-root path without wildcards and dots");
         return new (ctx) ast::error_expression(alias);
      }
   } else if (const ast::target_ref* tr = ast::as<ast::target_ref>(alias)) {
      if (tr->is_public() || !tr->build_request().empty() || tr->target_name().valid()) {
         diag.error(tr->start_loc(), "Argument 'alias': Expected non-public target reference expression without target name and build request");
         return new (ctx) ast::error_expression(alias);
      }
   } else {
      diag.error(alias->start_loc(), "Argument 'alias': Expected target or id or path expression");
      return new (ctx) ast::error_expression(alias);
   }

   return alias;
}

static
const ast::expression*
use_project_match_validator(ast::context& ctx,
                            diagnostic& diag,
                            const ast::expression* match_) {
   const ast::id_expr* match = ast::as<ast::id_expr>(match_);
   if (!match) {
      diag.error(match_->start_loc(), "Argument 'match': Expected identifier");
      return new (ctx) ast::error_expression(match_);
   }

   if (match->id().to_string() == "always" || match->id().to_string() == "exact")
      return match_;

   diag.error(match_->start_loc(), "Argument 'match': Valid values are: always, exact");
   return new (ctx) ast::error_expression(match_);
}

static
void use_project_rule(invocation_context& ctx,
                      const ast::expression* alias,
                      const location_t& location,
                      feature_set* requirements,
                      const parscore::identifier* i_match_strategy)
{
   location_t project_alias;
   project::alias::match match_strategy = project::alias::match::always;

   if (i_match_strategy) {
      const std::string ms = i_match_strategy->to_string();
      if (ms == "always")
         match_strategy = project::alias::match::always;
      else if (ms == "exact")
         match_strategy = project::alias::match::exact;
      else
         assert(false);
   }

   if (alias) {
      if (const ast::id_expr* id = ast::as<ast::id_expr>(alias))
         project_alias = location_t{id->id().to_string()};
      else if (const ast::path* p = ast::as<ast::path>(alias))
         project_alias = location_t{p->to_string()};
      else {
         const ast::target_ref* tr = ast::as<ast::target_ref>(alias);
         // FIXME: we need a way to move this check upper, to ast processing
         if (tr->target_path()->root_name().valid() && ctx.current_project_.is_root())
            throw ast2objects_semantic_error(alias->start_loc(), "Argument 'alias': Global aliases can be declared only in hamroot file");

         project_alias = location_t{tr->target_path()->to_string()};
      }
   }

   if (project_alias.has_root_path())
      ctx.current_project_.get_engine().add_alias(project_alias, ctx.current_project_.location() / location, requirements, match_strategy);
   else
      ctx.current_project_.add_alias(project_alias, location, requirements, match_strategy);
}

static
boost::unordered_set<std::string>
valid_feature_attributes = {
   "propagated", "composite", "free", "path",
   "incidental", "optional",
   "dependency", "no-defaults", "no-checks",
   "generated", "ordered"
};

// FIXME: we need to create ast::error_expression instead of just diagnose errors
// FIXME: handle duplicates
static
const ast::expression*
feature_attributes_validator(ast::context& ctx,
                             diagnostic& diag,
                             const ast::expression* e)
{
   auto handle_one = [&](const ast::expression* e) {
      if (const ast::id_expr* id = ast::as<ast::id_expr>(e)) {
         if (valid_feature_attributes.find(id->id().to_string()) == valid_feature_attributes.end())
            diag.error(id->start_loc(), "Invalid feature attribute");
      }
   };

   if (const ast::list_of* l = ast::as<ast::list_of>(e)) {
      for (const ast::expression* v : l->values())
         handle_one(v);

      return e;
   } else
      handle_one(e);

   return e;
}

static
feature_attributes
resolve_attributes(const one_or_list<parscore::identifier>* attributes)
{

   feature_attributes result = {0};
   if (!attributes)
      return result;

   auto i = find(attributes->begin(), attributes->end(), "propagated");
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

   i = find(attributes->begin(), attributes->end(), "dependency");
   if (i != attributes->end())
      result.dependency = true;

   i = find(attributes->begin(), attributes->end(), "no-defaults");
   if (i != attributes->end())
      result.no_defaults = true;

   i = find(attributes->begin(), attributes->end(), "no-checks");
   if (i != attributes->end())
      result.no_checks = true;

   i = find(attributes->begin(), attributes->end(), "ordered");
   if (i != attributes->end())
      result.ordered = true;

   return result;
}

static
std::vector<std::string>
to_simple_ids(const one_or_list<parscore::identifier>* values)
{
   std::vector<std::string> result;

   if (!values)
      return result;

   transform(values->begin(), values->end(), back_inserter(result), [&](const parscore::identifier& v){
      return v.to_string();
   });

   return result;
}

static
void feature_rule(invocation_context& ctx,
                  const parscore::identifier& name,
                  const one_or_list<parscore::identifier>* values_,
                  const one_or_list<parscore::identifier>* attributes)
{
   std::vector<std::string> values = to_simple_ids(values_);
   feature_def::legal_values_t legal_values;
   transform(values.begin(), values.end(), back_inserter(legal_values), [](const string& v) -> feature_def::legal_value {
      return { v, {} };
   });

   ctx.current_project_.feature_registry().add_feature_def(name.to_string(), legal_values, resolve_attributes(attributes));
}

static
void feature_compose_rule(invocation_context& ctx,
                          const feature& f,
                          const feature_or_feature_set_t& components)
{
   feature_registry& fr = ctx.current_project_.feature_registry();
   feature_set* cc;
   if (auto f = boost::get<const feature*>(&components)) {
      cc = fr.make_set();
      cc->join(**f);
   } else
      cc = (**boost::get<const feature_set*>(&components)).clone();

   fr.get_def(f.definition().name()).compose(f.value(), cc);
}

static
void feature_subfeature_rule(invocation_context& ctx,
                             const parscore::identifier& feature_name,
                             const parscore::identifier& subfeature_name)
{
   feature_def& def = ctx.current_project_.feature_registry().get_def(feature_name.to_string());
   def.add_subfeature(subfeature_name.to_string());
}

static
void variant_rule(invocation_context& ctx,
                  const parscore::identifier& variant_name,
                  const parscore::identifier* base,
                  feature_set& components)
{
   feature_registry& fr = ctx.current_project_.feature_registry();
   feature_def& def = fr.get_def("variant");
   def.extend_legal_values(variant_name.to_string(), {});

   if (base) {
      feature_set* composite_features = fr.make_set();
      def.expand_composites(base->to_string(), composite_features);
      composite_features->join(components);
      def.compose(variant_name.to_string(), composite_features);
   } else
      def.compose(variant_name.to_string(), &components);
}

static
void glob_impl(sources_decl& result,
               const fs::path& searching_path,
               const fs::path& relative_path,
               const boost::dos_wildcard& wildcard,
               const std::vector<std::string>* exceptions,
               const project& p)
{
   for (fs::directory_iterator i(searching_path), last = fs::directory_iterator(); i != last; ++i) {
      if (!is_directory(*i) && wildcard.match(i->path().filename()) &&
          !(exceptions != 0 && find(exceptions->begin(), exceptions->end(), i->path().filename().string()) != exceptions->end()))
      {
         result.push_back(p, (relative_path / i->path().filename()).string(), p.get_engine().get_type_registry());
      }
   }
}

static
void rglob_impl(sources_decl& result,
                const fs::path& searching_path,
                fs::path relative_path,
                const boost::dos_wildcard& wildcard,
                const std::vector<std::string>* exceptions,
                const project& p)
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
         result.push_back(p, (relative_path / i->path().filename()).string(), p.get_engine().get_type_registry());
      }
   }
}

static
const ast::expression*
glob_patterns_validator(ast::context& ctx,
                        diagnostic& diag,
                        const ast::expression* e)
{
   auto check_path = [&](const ast::expression* e) -> const ast::expression* {
      if (const ast::path* p = ast::as<ast::path>(e)) {
         if (!p->has_wildcard()) {
            diag.error(e->start_loc(), "Path with wildcard expected");
            return new (ctx) ast::error_expression(e);
         }
      }

      return e;
   };

   if (const ast::list_of* l = ast::as<ast::list_of>(e)) {
      ast::expressions_t values(ast::expressions_t::allocator_type{ctx});
      for (const ast::expression* el : l->values())
         values.push_back(check_path(el));

      return new (ctx) ast::list_of(values);
   }

   return e;
}

static
std::unique_ptr<sources_decl>
glob_rule_impl(invocation_context& ctx,
               const one_or_list<wcpath>& patterns,
               const one_or_list<parscore::identifier>* exceptions,
               const bool recursive)
{
   auto result = boost::make_unique<sources_decl>();
   std::vector<std::string> s_exceptions;
   if (exceptions)
      s_exceptions = to_simple_ids(exceptions);

   std::vector<location_t> l_patterns;
   transform(patterns.begin(), patterns.end(), back_inserter(l_patterns), [](const wcpath& p) { return p.to_location(); });

   for (auto& l_pattern : l_patterns ) {
      const string pattern = l_pattern.string();
      string::size_type mask_pos = pattern.find_first_of("*?");
      string::size_type separator_pos = pattern.find_last_of("/\\", mask_pos);
      fs::path relative_path(separator_pos == string::npos ? fs::path() : fs::path(pattern.begin(),
                                                                          pattern.begin() + separator_pos));
      fs::path searching_path(ctx.current_project_.location() / relative_path);
      boost::dos_wildcard wildcard(string(pattern.begin() + mask_pos, pattern.end()));
      if (recursive)
         rglob_impl(*result, searching_path, relative_path, wildcard, &s_exceptions, ctx.current_project_);
      else
         glob_impl(*result, searching_path, relative_path, wildcard, &s_exceptions, ctx.current_project_);
   }

   return result;
}

static
std::unique_ptr<sources_decl>
glob_rule(invocation_context& ctx,
          const one_or_list<wcpath>& patterns,
          const one_or_list<parscore::identifier>* exceptions)
{
   return glob_rule_impl(ctx, patterns, exceptions, false);
}

static
std::unique_ptr<sources_decl>
rglob_rule(invocation_context& ctx,
           const one_or_list<wcpath>& patterns,
           const one_or_list<parscore::identifier>* exceptions)
{
   return glob_rule_impl(ctx, patterns, exceptions, true);
}

static
void copy_rule(target_invocation_context& ctx,
               const parscore::identifier& name,
               const sources_decl& sources,
               const location_t& destination,
               const one_or_list<parscore::identifier>& ast_types_to_copy,
               const parscore::identifier* ast_recursive)
{
   vector<const target_type*> types_to_copy;
   const type_registry& tr = ctx.current_project_.get_engine().get_type_registry();

   for (const parscore::identifier& id : ast_types_to_copy) {
      const target_type* type = tr.find(type_tag(id.to_string()));
      if (!type)
         throw ast2objects_semantic_error(id.start_loc(), "Argument 'types': Unknown type '" + id.to_string() + "'");
      else
         types_to_copy.push_back(type);
   }

   bool recursive = false;
   if (ast_recursive) {
      if (ast_recursive->to_string() != "true" && ast_recursive->to_string() != "false")
         throw ast2objects_semantic_error(ast_recursive->start_loc(), "Argument 'recursive': Use 'true' or 'false' constants");

      recursive = (ast_recursive->to_string() == "true");
   }

   unique_ptr<basic_meta_target> mt(new copy_meta_target(&ctx.current_project_,
                                                         name.to_string(),
                                                         destination,
                                                         types_to_copy,
                                                         recursive));

   mt->sources(sources);
   ctx.current_project_.add_target(move(mt));
}

static
void obj_rule(target_invocation_context& ctx,
              const parscore::identifier& name,
              const sources_decl& sources,
              const requirements_decl* requirements,
              const requirements_decl* default_build,
              const usage_requirements_decl* usage_requirements)
{
   unique_ptr<basic_meta_target> mt(new obj_meta_target(&ctx.current_project_,
                                                        name.to_string(),
                                                        requirements ? *requirements : requirements_decl(),
                                                        usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   mt->sources(sources);
   if (default_build)
      mt->default_build(*default_build);
   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(move(mt));
}

static
void testing_suite_rule(target_invocation_context& ctx,
                        const parscore::identifier& name,
                        const sources_decl& sources,
                        const sources_decl* common_sources,
                        const requirements_decl* common_requirements)
{
   auto mt = make_unique<testing_suite_meta_target>(ctx.current_project_,
                                                    name.to_string(),
                                                    sources,
                                                    common_sources ? *common_sources : sources_decl{},
                                                    common_requirements ? *common_requirements : requirements_decl{});

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(move(mt));
}

static
testing_run_meta_target::args
make_testing_run_args(target_invocation_context& ctx,
                      const ast::expression* ast_args)
{
   testing_run_meta_target::args result;

   if (!ast_args)
      return result;

   auto handle_element = [&](const ast::expression* e) {
      if (const ast::id_expr* id = ast::as<ast::id_expr>(e))
         result.push_back(id->id().to_string());
      else if (const ast::path* p = ast::as<ast::path>(e)) {
         if (!p->has_wildcard())
            result.push_back(fs::path(p->to_string()));
         else
            throw ast2objects_semantic_error(p->start_loc(), "Paths with wildcard elements are not supported here");
      } else
         throw ast2objects_semantic_error(e->start_loc(), "Unexpected type");
   };

   if (const ast::list_of* l = ast::as<ast::list_of>(ast_args)) {
      for (const ast::expression* e : l->values())
         handle_element(e);
   } else
      handle_element(ast_args);

   return result;
}

static
std::string testing_make_target_name(const sources_decl& sources,
                                     const parscore::identifier* user_provided_target_name) {
   if (user_provided_target_name)
      return user_provided_target_name->to_string();
   else {
      auto first_src = *sources.begin();
      if (!fs::path(first_src.target_path()).has_extension() || !first_src.target_name().empty()) {
         // ok, user doesn't need readable name
         return to_string(boost::uuids::random_generator{}());
      } else
         return fs::path(first_src.target_path()).stem().string();
   }
}

static
std::unique_ptr<sources_decl>
testing_run_rule_impl(target_invocation_context& ctx,
                      const sources_decl& sources,
                      const requirements_decl* requirements,
                      const ast::expression* ast_args,
                      const string& target_name,
                      const bool recheck)
{
   const string exe_target_name = target_name + ".run";
   unique_ptr<basic_meta_target> intermediate_exe(
      new testing_intermediate_meta_target(&ctx.current_project_,
                                           exe_target_name,
                                           requirements ? *requirements : requirements_decl()));

   intermediate_exe->sources(sources);
   intermediate_exe->set_local(true);
   intermediate_exe->set_explicit(true);

   ctx.current_project_.add_target(move(intermediate_exe));

   auto runner_target =
      make_unique<testing_run_meta_target>(&ctx.current_project_,
                                           target_name,
                                           make_testing_run_args(ctx, ast_args),
                                           recheck);

   sources_decl run_sources;
   run_sources.push_back({ctx.current_project_, "./", exe_target_name, nullptr});
   runner_target->sources(run_sources);
   runner_target->set_local(ctx.local_);
   runner_target->set_explicit(ctx.explicit_);

   source_decl run_target_source(ctx.current_project_,
                                 runner_target->name(),
                                 {},
                                 nullptr /*to signal that this is meta target*/);

   ctx.current_project_.add_target(move(runner_target));

   auto result = boost::make_unique<sources_decl>();
   result->push_back(run_target_source);

   return result;
}

static
const ast::expression*
testing_run_recheck_validator(ast::context& ctx,
                              diagnostic& diag,
                              const ast::expression* recheck_) {
   const ast::id_expr* recheck = ast::as<ast::id_expr>(recheck_);
   if (!recheck) {
      diag.error(recheck_->start_loc(), "Argument 'recheck': Expected identifier");
      return new (ctx) ast::error_expression(recheck_);
   }

   auto value = recheck->id().to_string();
   if (value == "true" || value == "false")
      return recheck_;

   diag.error(recheck_->start_loc(), "Argument 'recheck': Expected 'true' or 'false'");
   return new (ctx) ast::error_expression(recheck_);
}

static
std::unique_ptr<sources_decl>
testing_run_rule(target_invocation_context& ctx,
                 const sources_decl& sources,
                 const requirements_decl* requirements,
                 const ast::expression* ast_args,
                 const parscore::identifier* user_provided_target_name,
                 const parscore::identifier* recheck_)
{
   const bool recheck = recheck_ && recheck_->to_string() == "true";
   return testing_run_rule_impl(ctx, sources, requirements, ast_args, testing_make_target_name(sources, user_provided_target_name), recheck);
}

static
std::unique_ptr<sources_decl>
testing_run_many_rule(invocation_context& ctx,
                      const sources_decl& sources,
                      const sources_decl* common_sources,
                      const requirements_decl* requirements,
                      const ast::expression* ast_args,
                      const parscore::identifier* name_template,
                      const parscore::identifier* recheck)
{
   auto result = boost::make_unique<sources_decl>();
   target_invocation_context tctx{ ctx, true, true };
   for (const source_decl& sd : sources) {
      sources_decl src;
      src.push_back(sd);
      if (common_sources)
         src.insert(*common_sources);

      const string target_name = [&](){
         const string tn = testing_make_target_name(src, nullptr);
         if (name_template)
            return boost::replace_all_copy(name_template->to_string(), "${name}", tn);
         else
            return tn;
      }();

      auto sd_run = testing_run_rule_impl(tctx, src, requirements, ast_args, target_name, recheck && recheck->to_string() == "true");
      assert(sd_run->size() == 1);

      result->insert(*sd_run);
   }

   return result;
}

static
std::unique_ptr<sources_decl>
testing_compile_fail_rule(target_invocation_context& ctx,
                          const sources_decl& sources,
                          const requirements_decl* requirements,
                          const parscore::identifier* user_provided_target_name)
{
   const string target_name = user_provided_target_name ? user_provided_target_name->to_string()
                                                        :  location_t(sources.begin()->target_path()).stem().string();
   unique_ptr<basic_meta_target> mt(new testing_compile_fail_meta_target(&ctx.current_project_,
                                                                         target_name,
                                                                         requirements ? *requirements : requirements_decl()));
   mt->sources(sources);

   const source_decl compile_source(ctx.current_project_,
                                    mt->name(),
                                    std::string(),
                                    nullptr /*to signal that this is meta target*/);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(move(mt));

   auto result = boost::make_unique<sources_decl>();
   result->push_back(compile_source);

   return result;
}

static
std::unique_ptr<sources_decl>
testing_compile_rule_impl(target_invocation_context& ctx,
                          const sources_decl& sources,
                          const requirements_decl* requirements,
                          const string& target_name)
{
   unique_ptr<basic_meta_target> mt(new testing_compile_meta_target(&ctx.current_project_,
                                                                    target_name,
                                                                    requirements ? *requirements : requirements_decl{}));
   mt->sources(sources);

   const source_decl compile_source(ctx.current_project_,
                                    mt->name(),
                                    std::string(),
                                    nullptr /*to signal that this is meta target*/);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(move(mt));

   auto result = boost::make_unique<sources_decl>();
   result->push_back(compile_source);

   return result;
}

static
std::unique_ptr<sources_decl>
testing_compile_rule(target_invocation_context& ctx,
                     const sources_decl& sources,
                     const requirements_decl* requirements,
                     const parscore::identifier* user_provided_target_name)
{
   return testing_compile_rule_impl(ctx, sources, requirements, testing_make_target_name(sources, user_provided_target_name));
}

static
std::unique_ptr<sources_decl>
testing_compile_many_rule(invocation_context& ctx,
                          const sources_decl& sources,
                          const sources_decl* common_sources,
                          const requirements_decl* requirements,
                          const parscore::identifier* name_template)
{
   auto result = boost::make_unique<sources_decl>();

   target_invocation_context tctx{ ctx, true, true };
   for (const source_decl& sd : sources) {
      sources_decl src;
      src.push_back(sd);
      if (common_sources)
         src.insert(*common_sources);

      const string target_name = [&](){
         const string tn = testing_make_target_name(src, nullptr);
         if (name_template)
            return boost::replace_all_copy(name_template->to_string(), "${name}", tn);
         else
            return tn;
      }();

      auto sd_run = testing_compile_rule_impl(tctx, src, requirements, target_name);
      assert(sd_run->size() == 1);

      result->insert(*sd_run);
   }

   return result;
}

static
std::unique_ptr<sources_decl>
testing_link_fail_rule(target_invocation_context& ctx,
                       const sources_decl& sources,
                       const requirements_decl* requirements,
                       const parscore::identifier* user_provided_target_name)
{
   const string target_name = user_provided_target_name ? user_provided_target_name->to_string()
                                                        :  location_t(sources.begin()->target_path()).stem().string();
   unique_ptr<basic_meta_target> mt(new testing_link_fail_meta_target(&ctx.current_project_,
                                                                      target_name,
                                                                      requirements ? *requirements : requirements_decl()));
   mt->sources(sources);

   const source_decl compile_source(ctx.current_project_,
                                    mt->name(),
                                    std::string(),
                                    nullptr /*to signal that this is meta target*/);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(move(mt));

   auto result = boost::make_unique<sources_decl>();
   result->push_back(compile_source);

   return result;
}

static
std::unique_ptr<sources_decl>
testing_link_rule(target_invocation_context& ctx,
                  const sources_decl& sources,
                  const requirements_decl* requirements,
                  const parscore::identifier* user_provided_target_name)
{
   const string target_name = user_provided_target_name ? user_provided_target_name->to_string()
                                                        :  location_t(sources.begin()->target_path()).stem().string();
   unique_ptr<basic_meta_target> mt(new testing_link_meta_target(&ctx.current_project_,
                                                                 target_name,
                                                                 requirements ? *requirements : requirements_decl{}));
   mt->sources(sources);

   const source_decl compile_source(ctx.current_project_,
                                    mt->name(),
                                    std::string(),
                                    nullptr /*to signal that this is meta target*/);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(move(mt));

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
   unique_ptr<basic_meta_target> mt(new prebuilt_lib_meta_target(&ctx.current_project_,
                                                                 name.to_string(),
                                                                 filename.string(),
                                                                 requirements ? *requirements : requirements_decl(),
                                                                 usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl()));
   if (sources)
      mt->sources(*sources);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(move(mt));
}

static
void searched_shared_lib_rule(target_invocation_context& ctx,
                              const parscore::identifier& name,
                              const sources_decl* sources,
                              const parscore::identifier& libname,
                              const requirements_decl* requirements,
                              const usage_requirements_decl* usage_requirements)
{
   unique_ptr<basic_meta_target> mt(new searched_lib_meta_target(&ctx.current_project_,
                                                                 name.to_string(),
                                                                 libname.to_string(),
                                                                 requirements ? *requirements : requirements_decl(),
                                                                 usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl(),
                                                                 ctx.current_project_.get_engine().get_type_registry().get(types::SEARCHED_SHARED_LIB)));
   if (sources)
      mt->sources(*sources);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(move(mt));
}

static
void searched_static_lib_rule(target_invocation_context& ctx,
                              const parscore::identifier& name,
                              const sources_decl* sources,
                              const parscore::identifier& libname,
                              const requirements_decl* requirements,
                              const usage_requirements_decl* usage_requirements)
{
   unique_ptr<basic_meta_target> mt(new searched_lib_meta_target(&ctx.current_project_,
                                                                 name.to_string(),
                                                                 libname.to_string(),
                                                                 requirements ? *requirements : requirements_decl(),
                                                                 usage_requirements ? static_cast<const requirements_decl&>(*usage_requirements) : requirements_decl(),
                                                                 ctx.current_project_.get_engine().get_type_registry().get(types::SEARCHED_STATIC_LIB)));
   if (sources)
      mt->sources(*sources);

   mt->set_local(ctx.local_);
   mt->set_explicit(ctx.explicit_);
   ctx.current_project_.add_target(move(mt));
}

static
void setup_warehouse_rule(invocation_context& ctx,
                          const parscore::identifier& id,
                          const parscore::identifier& url,
                          const location_t* storage_dir_)
{
   fs::path storage_dir = [&] {
      if (storage_dir_ && !storage_dir_->empty())
         return *storage_dir_;
      else
         return fs::path();
   }();

   auto& whm = ctx.current_project_.get_engine().warehouse_manager();
   auto sid = id.to_string();
   if (whm.find(sid) != whm.end())
      throw std::runtime_error("Warehouse '" + sid + "' already registered");

   unique_ptr<warehouse> wh{ new warehouse_impl{ ctx.current_project_.get_engine(), sid, url.to_string(), storage_dir } };
   ctx.current_project_.get_engine().warehouse_manager().insert(std::move(wh));
}

static
std::unique_ptr<sources_decl>
c_as_cpp_rule(invocation_context& ctx,
              const sources_decl& sources)
{
   auto result = boost::make_unique<sources_decl>();
   const target_type& c_as_cpp_type = ctx.current_project_.get_engine().get_type_registry().get(types::C_AS_CPP);
   const target_type& c_type = ctx.current_project_.get_engine().get_type_registry().get(types::C);
   for (const source_decl& s : sources) {
      if (s.type() && s.type()->equal_or_derived_from(c_type)) {
         auto new_sd = s;
         new_sd.set_type(&c_as_cpp_type);
         result->push_back(new_sd);
      } else
         result->push_back(s);
   }

   return result;
}

void install_builtin_rules(rule_manager& rm)
{
   rm.add_rule("project", project_rule, {{"id", project_id_validator}, "requirements", "usage-requirements", "dependencies", {"default-build", default_build_checker}});
   rm.add_rule("use-project", use_project_rule, {{"alias", use_project_alias_validator}, "location", "requirements", {"match", use_project_match_validator}});
   rm.add_rule("feature", feature_rule, {"name", "values", {"attributes", feature_attributes_validator}});
   rm.add_rule("feature.compose", feature_compose_rule, {"feature", "components"});
   rm.add_rule("feature.subfeature", feature_subfeature_rule, {"feature-name", "subfeature-name"});
   rm.add_rule("variant", variant_rule, {"name", "base", "components"});
   rm.add_rule("glob", glob_rule, {{"patterns", glob_patterns_validator}, "exceptions"});
   rm.add_rule("rglob", rglob_rule, {{"patterns", glob_patterns_validator}, "exceptions"});
   rm.add_target("exe", exe_rule, {"name", "sources", "requirements", {"default-build", default_build_checker}, "usage-requirements"});
   rm.add_target("lib", lib_rule, {"name", "sources", "requirements", {"default-build", default_build_checker}, "usage-requirements"});
   rm.add_target("alias", alias_rule, {"name", "sources", "requirements", "usage-requirements"});
   rm.add_target("header-lib", header_lib_rule, {"name", "sources", "requirements", {"default-build", default_build_checker}, "usage-requirements"});
   rm.add_target("prebuilt-lib", prebuilt_lib_rule, {"name", "sources", "filename", "requirements", "usage-requirements"});
   rm.add_target("searched-shared-lib", searched_shared_lib_rule, {"name", "sources", "libname", "requirements", "usage-requirements"});
   rm.add_target("searched-static-lib", searched_static_lib_rule, {"name", "sources", "libname", "requirements", "usage-requirements"});
   rm.add_target("copy", copy_rule, {"name", "sources", "destination", "types", "recursive"});
   rm.add_target("obj", obj_rule, {"name", "sources", "requirements", {"default-build", default_build_checker}, "usage-requirements"});
   rm.add_target("testing.suite", testing_suite_rule, {"name", "sources", "common-sources", "common-requirements"});
   rm.add_target("testing.run", testing_run_rule, {"sources", "requirements", "args", "name", { "recheck", testing_run_recheck_validator }});
   rm.add_rule("testing.run-many", testing_run_many_rule, {"sources", "common-sources", "requirements", "args", "name-template", { "recheck", testing_run_recheck_validator }});
   rm.add_target("testing.compile-fail", testing_compile_fail_rule, {"sources", "requirements", "name"});
   rm.add_target("testing.compile", testing_compile_rule, {"sources", "requirements", "name"});
   rm.add_rule("testing.compile-many", testing_compile_many_rule, {"sources", "common-sources", "requirements", "name-template"});
   rm.add_target("testing.link-fail", testing_link_fail_rule, {"sources", "requirements", "name"});
   rm.add_target("testing.link", testing_link_rule, {"sources", "requirements", "name"});
   rm.add_rule("setup-warehouse", setup_warehouse_rule, {"name", "url", "storage-dir"});
   rm.add_rule("c-as-cpp", c_as_cpp_rule, {"sources"});
}

}}
