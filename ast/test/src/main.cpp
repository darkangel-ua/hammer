#include <boost/test/auto_unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/bind.hpp>
#include <boost/optional/optional.hpp>
#include <iostream>
#include <hammer/parser/parser.h>
#include <hammer/sema/actions_impl.h>
#include <hammer/ast/context.h>
#include <hammer/ast/ast_xml_printer.h>
#include <hammer/ast/hamfile.h>
#include <hammer/ast/list_of.h>
#include <hammer/ast/casts.h>
#include <hammer/core/rule_manager.h>
#include <hammer/core/diagnostic.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <set>

boost::filesystem::path test_data_path;

using namespace std; 
using namespace hammer;
using namespace boost::unit_test;
namespace fs = boost::filesystem;

class test_requirements;
class test_sources{};
class test_feature_set;
class test_path;
class test_wcpath;
class test_usage_requirements;
class test_feature;
class test_target_ref;
class test_target_ref_mask;

struct struct_1 {
   parscore::identifier pattern_;
   parscore::identifier version_;
};

struct struct_2 {
   parscore::identifier pattern_;
   boost::optional<parscore::identifier> version_;
};

namespace hammer {
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(test_requirements, requirement_set);
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(test_sources, sources);
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(test_feature_set, feature_set);
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(test_feature, feature);
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(test_path, path);
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(vector<test_path>, path_or_list_of_paths);
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(test_wcpath, wcpath);
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(vector<test_wcpath>, wcpath_or_list_of_wcpaths);
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(test_usage_requirements, usage_requirements);
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(test_target_ref, target_ref);
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(test_target_ref_mask, target_ref_mask);

   template<>
   struct rule_argument_type_info<struct_1> {
      static
      std::unique_ptr<struct_1>
      constructor(const parscore::identifier& pattern,
                  const parscore::identifier& version) {
         return std::unique_ptr<struct_1>{new struct_1{pattern, version}};
      }

      static
      rule_argument_type_desc
      ast_type() {
         return make_rule_argument_struct_desc("struct_1", constructor, {"pattern", "version"});
      }
   };

   template<>
   struct rule_argument_type_info<struct_2> {
      static
      std::unique_ptr<struct_2>
      constructor(const parscore::identifier& pattern,
                  const parscore::identifier* version) {
         return std::unique_ptr<struct_2>{new struct_2{pattern, version ? boost::optional<parscore::identifier>{} : boost::optional<parscore::identifier>{*version}}};
      }

      static
      rule_argument_type_desc
      ast_type() {
         return make_rule_argument_struct_desc("struct_2", constructor, {"pattern", "version"});
      }
   };
}

static
void project_rule(invocation_context& ctx,
                  const parscore::identifier& id,
                  const test_requirements* requirements,
                  const test_requirements* usage_requirements)
{
}

static
void lib_rule(target_invocation_context& ctx,
              const parscore::identifier& id,
              const test_sources* sources,
              const test_requirements* requirements,
              const test_feature_set* default_build,
              const test_requirements* usage_requirements)
{
}

static
void exe_rule(target_invocation_context& ctx,
              const parscore::identifier& id,
              const test_sources* sources,
              const test_requirements* requirements,
              const test_feature_set* default_build,
              const test_requirements* usage_requirements)
{
}

static
std::unique_ptr<test_sources>
glob_rule(invocation_context& ctx,
          const test_wcpath& pattern)
{
   return {};
}

static
std::unique_ptr<test_sources>
rglob_rule(invocation_context& ctx,
           const test_wcpath& pattern)
{
   return {};
}

static
void
feature_rule(invocation_context& ctx,
             const parscore::identifier& name)
{

}

static
void requirements_test_rule(invocation_context& ctx,
                            const test_requirements& requirements)
{
}

static
void usage_requirements_test_rule(invocation_context& ctx,
                                  const test_usage_requirements& requirements)
{
}

static
void feature_test_rule(invocation_context& ctx,
                       const test_feature& f)
{
}

static
void feature_set_test_rule(invocation_context& ctx,
                           const test_feature_set& f)
{
}

static
void sources_test_rule(invocation_context& ctx,
                       const test_sources&)
{
}

static
void path_test_rule(invocation_context& ctx,
                    const test_path&)
{
}

static
void paths_test_rule(invocation_context& ctx,
                     const vector<test_path>&)
{
}

static
void wcpath_test_rule(invocation_context& ctx,
                      const test_wcpath&)
{
}

static
void wcpaths_test_rule(invocation_context& ctx,
                       const vector<test_wcpath>&)
{
}

static
void target_ref_test_rule(invocation_context& ctx,
                          const test_target_ref&)
{
}

static
void target_ref_mask_test_rule(invocation_context& ctx,
                               const test_target_ref_mask&)
{
}

static
void struct_1_test_rule(invocation_context&,
                        const struct_1& s) {
}

static
void struct_2_test_rule(invocation_context&,
                        const struct_2&) {
}

static
const ast::expression*
user_defined_ast_transformer_test_transformer(ast::context& ctx,
                                              diagnostic& diag,
                                              const ast::expression* e)
{
   auto handle_one = [&](const ast::expression* e) -> const::ast::expression* {
      if (const ast::id_expr* id = ast::as<ast::id_expr>(e)) {
         if (id->id().to_string() == "zoo") {
            diag.error(id->start_loc(), "'zoo' is not allowed here");
            return new (ctx) ast::error_expression(id);
         } else
            return e;
      } else {
         diag.error(e->start_loc(), "Only simple identifier or string expected here");
         return new (ctx) ast::error_expression(e);
      }
   };

   if (const ast::list_of* l = ast::as<ast::list_of>(e)) {
      ast::expressions_t nvs(ast::expressions_t::allocator_type{ctx});
      for (const ast::expression* v : l->values())
         nvs.push_back(handle_one(v));

      return new (ctx) ast::list_of(nvs);
   } else
      return handle_one(e);
}

static
void user_defined_ast_transformer_test_rule(invocation_context& ctx,
                                            const ast::expression& e)
{
}

typedef map<int, pair<string, diagnostic::type::value> > expected_diags_t;

class checked_diagnostic : public diagnostic
{
   public:               
      checked_diagnostic(const std::string& source_name,
                         const expected_diags_t& expected_diags)
         : diagnostic(source_name, false),
           expected_diags_(expected_diags)
      {
      }
      void report_unreported_diagnostics() const;     
      size_t expected_error_count() const { return expected_diags_.size(); }

   private:
      typedef map<int, string> reported_lines_t;

      expected_diags_t expected_diags_;
      reported_lines_t reported_lines_;

      void report(const char* formated_message) override;
};

void checked_diagnostic::report(const char* formated_message)
{
   boost::cmatch m;
   if (regex_search(formated_message, m, boost::regex(R"(.+:(\d+):\d+: (\w+): (.*)\n)"))) {
      // m[1] line number
      // m[2] message type
      // m[3] message
      int line_number = boost::lexical_cast<int>(m[1]);
      
      diagnostic::type::value type;
      if (m[2] == "error")
         type = diagnostic::type::error;
      else
         BOOST_CHECK(false && "Unknown diagnostic type");
      
      expected_diags_t::const_iterator i = expected_diags_.find(line_number);
      if (i == expected_diags_.end())
         BOOST_CHECK_MESSAGE(false, "Unexpected diagnostic at line " + to_string(line_number) + " : " + m[3]);
      else {
         reported_lines_.insert({line_number, formated_message});
         BOOST_CHECK_EQUAL(i->second.second, type);
         BOOST_CHECK_MESSAGE(i->second.first == m[3], "[" + i->second.first + "!=" + m[3] + "] line number " + to_string(line_number));
      }
   } else
      BOOST_CHECK(false && "Unknown diagnostic format");
}

void checked_diagnostic::report_unreported_diagnostics() const
{
   set<int> covered_lines;

   for (const auto& i : expected_diags_) {
      if (reported_lines_.find(i.first) == reported_lines_.end())
         BOOST_CHECK_MESSAGE(false, string("Diagnostic '") + i.second.first + "' was not reported at line " + to_string(i.first));
      else
         covered_lines.insert(i.first);
   }

   for (const auto& line : reported_lines_) {
      if (covered_lines.find(line.first) == covered_lines.end())
         BOOST_CHECK_MESSAGE(false, line.second);
   }
}

static expected_diags_t extract_expected_diags(const fs::path& hamfile)
{
   expected_diags_t result;

   fs::ifstream stream(hamfile); 
   string line;
   boost::smatch m; 
   int line_number = 1;
   while(getline(stream, line))
   {
      if (regex_search(line, m, boost::regex(".*?#(\\w+): (.+)")))
         result[line_number] = make_pair(m[2], diagnostic::type::error);

      ++line_number;
   }

   return result;
}

struct sema_env : sema::actions_impl::environment {
   bool known_feature(const parscore::identifier& name) const override {
      return name.to_string() != "foo";
   }
};

void test_function(const fs::path& hamfile)
{
   rule_manager rule_manager;
   checked_diagnostic diag(hamfile.string(), extract_expected_diags(hamfile));

   rule_manager.add_rule("project", project_rule, {"project-name", "requirements", "usage-requirements"});
   rule_manager.add_rule("feature", feature_rule, {"name"});
   rule_manager.add_target("lib", lib_rule, {"target-name", "sources", "requirements", "default-build", "usage-requirements"});
   rule_manager.add_target("exe", exe_rule, {"target-name", "sources", "requirements", "default-build", "usage-requirements"});

   rule_manager.add_rule("glob", glob_rule, {"pattern"});
   rule_manager.add_rule("rglob", rglob_rule, {"pattern"});
   rule_manager.add_rule("requirements_test", requirements_test_rule, {"requirements"});
   rule_manager.add_rule("usage_requirements_test",usage_requirements_test_rule, {"usage-requirements"});
   rule_manager.add_rule("feature_test", feature_test_rule, {"feature"});
   rule_manager.add_rule("feature_set_test", feature_set_test_rule, {"feature_set"});
   rule_manager.add_rule("sources_test", sources_test_rule, {"sources"});
   rule_manager.add_rule("user_definded_ast_transformer_test", user_defined_ast_transformer_test_rule, {{"id-list", user_defined_ast_transformer_test_transformer}});
   rule_manager.add_rule("path_test", path_test_rule, {"path"});
   rule_manager.add_rule("paths_test", paths_test_rule, {"paths"});
   rule_manager.add_rule("wcpath_test", wcpath_test_rule, {"path"});
   rule_manager.add_rule("wcpaths_test", wcpaths_test_rule, {"paths"});
   rule_manager.add_rule("target_ref_test", target_ref_test_rule, {"target_ref"});
   rule_manager.add_rule("target_ref_mask_test", target_ref_mask_test_rule, {"target_ref_mask"});
   rule_manager.add_rule("struct_1_test", struct_1_test_rule, {"struct_arg"});
   rule_manager.add_rule("struct_2_test", struct_2_test_rule, {"struct_arg"});

   ast::context ctx;
   sema_env env;
   sema::actions_impl actions(ctx, env, rule_manager, diag);
   ast_hamfile_ptr ast_top = parse_hammer_script(hamfile, actions);
   
   BOOST_REQUIRE(ast_top);

   diag.report_unreported_diagnostics();

   fs::ofstream f(hamfile.branch_path() / (hamfile.filename().string() + ".ast"));
   ast::ast_xml_printer printer(f);
   ast_top->accept(printer);
}

boost::unit_test::test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
   if (argc < 2)
   {
      cout << "Need test data directory path\n";
      return NULL;
   }
       
   test_data_path = fs::path(argv[1]); 
   if (!test_data_path.has_root_path())
   {
      test_data_path = fs::current_path() / test_data_path;
      test_data_path.normalize();
   }

   for(fs::directory_iterator i(test_data_path); i != fs::directory_iterator(); ++i)
      if (i->path().filename() != ".svn" &&
          extension(i->path()) == ".ham")
      {
         framework::master_test_suite().add(make_test_case(boost::bind(&test_function, i->path()), basename(i->path())));
      }

   return 0;
}                             

