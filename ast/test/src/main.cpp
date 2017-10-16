#include "stdafx.h"
#include <boost/test/auto_unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/bind.hpp>
#include <boost/assign/std/vector.hpp>
#include <iostream>
#include <hammer/parser/parser.h>
#include <hammer/sema/actions_impl.h>
#include <hammer/ast/context.h>
#include <hammer/ast/ast_xml_printer.h>
#include <hammer/ast/hamfile.h>
#include <hammer/core/rule_manager.h>
#include <hammer/core/diagnostic.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <set>

boost::filesystem::path test_data_path;

using namespace std; 
using namespace hammer;
using namespace boost::unit_test;
using namespace boost::assign;
namespace fs = boost::filesystem;

namespace hammer {

// FIXME: just to fool glob and rglob
class sources_decl {};

}

static
void project_rule(const parscore::identifier& id,
                  const hammer::requirements_decl* requirements,
                  const hammer::requirements_decl* usage_requirements)
{

}

static void lib_rule(const parscore::identifier& id, 
                     const hammer::sources_decl* sources,
                     const hammer::requirements_decl* requirements,
                     const hammer::feature_set* default_build,
                     const hammer::requirements_decl* usage_requirements)
{

}

static void exe_rule(const parscore::identifier& id, 
                     const hammer::sources_decl* sources,
                     const hammer::requirements_decl* requirements,
                     const hammer::feature_set* default_build,
                     const hammer::requirements_decl* usage_requirements)
{

}

static
hammer::sources_decl
glob_rule(const hammer::path& pattern)
{
   return {};
}

static
hammer::sources_decl
rglob_rule(const hammer::path& pattern)
{
   return {};
}

static
void requirements_test_rule(const hammer::requirements_decl& requirements)
{

}

static
void usage_requirements_test_rule(const hammer::usage_requirements& requirements)
{

}

static
void feature_test_rule(const hammer::feature& f)
{

}

static
void feature_set_test_rule(const hammer::feature_set& f)
{

}

static
void sources_test_rule(const hammer::sources_decl&)
{

}

typedef map<int, pair<string, diagnostic::type::value> > expected_diags_t;

class checked_diagnostic : public diagnostic
{
   public:               
      checked_diagnostic(const std::string& source_name,
                         const expected_diags_t& expected_diags)
         : diagnostic(source_name),
           expected_diags_(expected_diags)
      {
      }
      void report_unreported_diagnostics() const;     
      size_t expected_error_count() const { return expected_diags_.size(); }

   private:
      typedef map<int, string> reported_lines_t;

      expected_diags_t expected_diags_;
      reported_lines_t reported_lines_;

      virtual void report(const char* formated_message);
};

void checked_diagnostic::report(const char* formated_message)
{
   boost::cmatch m;
   if (regex_search(formated_message, m, boost::regex(".+?\\((\\d+)\\) : (\\w+): (.*)"))) {
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

   for (const auto& i : expected_diags_)
      if (reported_lines_.find(i.first) == reported_lines_.end())
         BOOST_CHECK_MESSAGE(false, string("Diagnostic '") + i.second.first + "' was not reported");
      else
         covered_lines.insert(i.first);

   for (const auto& line : reported_lines_)
      if (covered_lines.find(line.first) == covered_lines.end())
         BOOST_CHECK_MESSAGE(false, line.second);
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

   {
      vector<parscore::identifier> arg_names;
      arg_names += "project-name", "requirements", "usage-requirements";
      rule_manager.add_target("project",
                              boost::function<void(const parscore::identifier&,
                                                   const hammer::requirements_decl*,
                                                   const hammer::requirements_decl*)>(&project_rule),
                              arg_names);
   }

   {
      vector<parscore::identifier> arg_names;
      arg_names += "target-name", "sources", "requirements", "default-build", "usage-requirements";
      rule_manager.add_target("lib", 
                              boost::function<void(const parscore::identifier&,
                                                   const hammer::sources_decl*,
                                                   const hammer::requirements_decl*,
                                                   const hammer::feature_set*,
                                                   const hammer::requirements_decl*)>(&lib_rule),
                              arg_names);
   }

   {
      vector<parscore::identifier> arg_names;
      arg_names += "target-name", "sources", "requirements", "default-build", "usage-requirements";
      rule_manager.add_target("exe", 
                              boost::function<void(const parscore::identifier&,
                                                   const hammer::sources_decl*,
                                                   const hammer::requirements_decl*,
                                                   const hammer::feature_set*,
                                                   const hammer::requirements_decl*)>(&exe_rule),
                              arg_names);
   }

   {
      vector<parscore::identifier> arg_names;
      arg_names += "pattern";
      rule_manager.add_target("glob", 
                              boost::function<hammer::sources_decl (const hammer::path&)>(&glob_rule),
                              arg_names);
   }

   {
      vector<parscore::identifier> arg_names;
      arg_names += "pattern";
      rule_manager.add_target("rglob", 
                              boost::function<hammer::sources_decl (const hammer::path&)>(&rglob_rule),
                              arg_names);
   }

   {
      vector<parscore::identifier> arg_names;
      arg_names += "requirements";
      rule_manager.add_target("requirements_test",
                              boost::function<void(const hammer::requirements_decl&)>(&requirements_test_rule),
                              arg_names);
   }

   {
      vector<parscore::identifier> arg_names;
      arg_names += "usage-requirements";
      rule_manager.add_target("usage_requirements_test",
                              boost::function<void(const hammer::usage_requirements&)>(&usage_requirements_test_rule),
                              arg_names);
   }

   {
      vector<parscore::identifier> arg_names;
      arg_names += "feature";
      rule_manager.add_target("feature_test",
                              boost::function<void(const hammer::feature&)>(&feature_test_rule),
                              arg_names);
   }

   {
      vector<parscore::identifier> arg_names;
      arg_names += "feature_set";
      rule_manager.add_target("feature_set_test",
                              boost::function<void(const hammer::feature_set&)>(&feature_set_test_rule),
                              arg_names);
   }

   {
      vector<parscore::identifier> arg_names;
      arg_names += "sources";
      rule_manager.add_target("sources_test",
                              boost::function<void(const hammer::sources_decl&)>(&sources_test_rule),
                              arg_names);
   }

   ast::context ctx(rule_manager, diag);
   sema::actions_impl actions(ctx);
   parser::parser::hamfile_ptr ast_top = parser::parser::parse(hamfile, actions);
   
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

