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

static void glob_rule(const hammer::path_like_seq& pattern)
{

}

static void rglob_rule(const hammer::path_like_seq& pattern)
{

}

typedef std::map<int, std::pair<std::string, diagnostic::type::value> > expected_diags_t;

class checked_diagnostic : public diagnostic
{
   public:               
      checked_diagnostic(const expected_diags_t& expected_diags)
         : expected_diags_(expected_diags)
      {
      }
      void report_unreported_diagnostics() const;     

   private:
      typedef std::set<int> checked_lines_t;

      expected_diags_t expected_diags_;
      checked_lines_t checked_lines_;

      virtual void report(const char* formated_message);
};

void checked_diagnostic::report(const char* formated_message)
{
   boost::cmatch m;
   if (regex_search(formated_message, m, boost::regex(".+?\\((\\d+)\\) : (\\w+): (.*)")))
   {
      // m[1] line number
      // m[2] message type
      // m[3] message
      int line = boost::lexical_cast<int>(m[1]);
      
      diagnostic::type::value type;
      if (m[2] == "error")
         type = diagnostic::type::error;
      else
         BOOST_CHECK(false && "Unknown diagnostic type");
      
      expected_diags_t::const_iterator i = expected_diags_.find(line);
      if (i == expected_diags_.end())
         BOOST_CHECK_MESSAGE(false, "Unexpected diagnostic: " + m[3]);
      else
      {
         checked_lines_.insert(line);
         BOOST_CHECK_EQUAL(i->second.second, type);
         BOOST_CHECK_EQUAL(i->second.first, m[3]);
      }
   }
   else
      BOOST_CHECK(false && "Unknown diagnostic format");
}

void checked_diagnostic::report_unreported_diagnostics() const
{
   for(expected_diags_t::const_iterator i = expected_diags_.begin(), last = expected_diags_.end(); i != last; ++i)
      if (checked_lines_.find(i->first) == checked_lines_.end())
         BOOST_CHECK_MESSAGE(false, string("Diagnostic '") + i->second.first + "' was not reported");
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
   checked_diagnostic diag(extract_expected_diags(hamfile));

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
                                                   const hammer::requirements_decl*)>(&lib_rule),
                              arg_names);
   }

   {
      vector<parscore::identifier> arg_names;
      arg_names += "pattern";
      rule_manager.add_target("glob", 
                              boost::function<void(const hammer::path_like_seq&)>(&glob_rule),
                              arg_names);
   }

   {
      vector<parscore::identifier> arg_names;
      arg_names += "pattern";
      rule_manager.add_target("rglob", 
                              boost::function<void(const hammer::path_like_seq&)>(&rglob_rule),
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

