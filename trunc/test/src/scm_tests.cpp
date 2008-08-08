#include "stdafx.h"
#include "enviroment.h"
#include <hammer/src/engine.h>
#include <boost/regex.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

struct scm_tests
{
   scm_tests()
   {
   }

   void load_project(const string& name);
   engine engine_;
};

void scm_tests::load_project(const string& name)
{
   fs::path project_path(test_data_path / "scm_tests" / name);
   BOOST_REQUIRE(exists(project_path / "jamroot"));
   const project& root_project = engine_.load_project(project_path);
   fs::ifstream f(project_path / "jamroot");
   string file_content;
   copy(istreambuf_iterator<char>(f), istreambuf_iterator<char>(), back_inserter(file_content));
   boost::smatch m;
   if (regex_search(file_content, m, boost::regex("instantiate:\\s*([/a-zA-Z0-9\\-]+)")))
   {
      for(boost::smatch::const_iterator i = ++m.begin(), last = m.end(); i != last; ++i)
         BOOST_CHECK_NO_THROW(engine_.load_project(location_t(*i), root_project));
   }
   else
      BOOST_FAIL("No instantiation instructions!");
}

BOOST_FIXTURE_TEST_CASE(svn_simple_checkout, scm_tests)
{
   BOOST_REQUIRE_NO_THROW(load_project("svn_simple_checkout"));
}