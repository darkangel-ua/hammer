#include "stdafx.h"
#include "enviroment.h"
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/process.hpp>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;
namespace bp = boost::process;

struct scm_tests
{
   scm_tests()
   {
   }

   void remove_all(const std::string& test_name)
   {
/*
      bp::launcher launcher;
      for(fs::directory_iterator i(test_data_path / "scm_tests" / test_name), last; i != last; ++i)
      {
         if (is_directory(*i) && i->filename() != ".svn")
            launcher.start(bp::command_line::shell("RMDIR /S /Q " + i->path().native_file_string())).wait();
      }
*/
   }

   void load_project(const string& name);
   engine engine_;
};

void scm_tests::load_project(const string& name)
{
   fs::path project_path(test_data_path / "scm_tests" / name);
   BOOST_REQUIRE(exists(project_path / "hamfile"));
   const project& p = engine_.load_project(project_path);
   std::vector<basic_target*> instantiated_targets;
   p.instantiate("test", *engine_.feature_registry().make_set(), &instantiated_targets);
}

/*
BOOST_FIXTURE_TEST_CASE(svn_simple_checkout, scm_tests)
{
   BOOST_REQUIRE_NO_THROW(remove_all("svn_simple_checkout"));
   BOOST_REQUIRE_NO_THROW(load_project("svn_simple_checkout"));
}

BOOST_FIXTURE_TEST_CASE(svn_complex_project, scm_tests)
{
   BOOST_REQUIRE_NO_THROW(remove_all("svn_complex_project"));
   BOOST_REQUIRE_NO_THROW(load_project("svn_complex_project"));
}

BOOST_FIXTURE_TEST_CASE(svn_repositories, scm_tests)
{
   BOOST_REQUIRE_NO_THROW(remove_all("svn_repositories"));
   BOOST_REQUIRE_NO_THROW(load_project("svn_repositories"));
   BOOST_CHECK(!exists(fs::path(test_data_path / "scm_tests/svn_repositories/rep2/boost/optional")));
}
*/
