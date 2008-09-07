#include "stdafx.h"
#include "enviroment.h"
#include <hammer/src/engine.h>
#include <hammer/src/feature_registry.h>
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
   const project& p = engine_.load_project(project_path);
   std::vector<basic_target*> result;
   p.instantiate("test", *engine_.feature_registry().make_set(), &result);
}

BOOST_FIXTURE_TEST_CASE(svn_simple_checkout, scm_tests)
{
   BOOST_REQUIRE_NO_THROW(load_project("svn_simple_checkout"));
}
