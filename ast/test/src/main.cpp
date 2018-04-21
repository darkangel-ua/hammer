#include <boost/test/auto_unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/bind.hpp>
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
class test_usage_requirements;
class test_feature;

namespace hammer {
   template<>
   struct rule_argument_type_info<test_requirements> { static const rule_argument_type ast_type = rule_argument_type::requirement_set; };
   template<>
   struct rule_argument_type_info<test_sources> { static const rule_argument_type ast_type = rule_argument_type::sources; };
   template<>
   struct rule_argument_type_info<test_feature_set> { static const rule_argument_type ast_type = rule_argument_type::feature_set; };
   template<>
   struct rule_argument_type_info<test_feature> { static const rule_argument_type ast_type = rule_argument_type::feature; };
   template<>
   struct rule_argument_type_info<test_path> { static const rule_argument_type ast_type = rule_argument_type::path; };
   template<>
   struct rule_argument_type_info<test_usage_requirements> { static const rule_argument_type ast_type = rule_argument_type::usage_requirements; };
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
          const test_path& pattern)
{
   return {};
}

static
std::unique_ptr<test_sources>
rglob_rule(invocation_context& ctx,
           const test_path& pattern)
{
   return {};
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
         BOOST_CHECK_EQUAL(i->second.first, m[3]);
      }
   } else
      BOOST_CHECK(false && "Unknown diagnostic format");
}

void checked_diagnostic::report_unreported_diagnostics() const
{
   set<int> covered_lines;

   for (const auto& i : expected_diags_) {
      if (reported_lines_.find(i.first) == reported_lines_.end())
         BOOST_CHECK_MESSAGE(false, string("Diagnostic '") + i.second.first + "' was not reported");
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

void test_function(const fs::path& hamfile)
{
   rule_manager rule_manager;
   checked_diagnostic diag(hamfile.string(), extract_expected_diags(hamfile));

   rule_manager.add_rule("project", project_rule, {"project-name", "requirements", "usage-requirements"});
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

   ast::context ctx;
   sema::actions_impl actions(ctx, rule_manager, diag);
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

