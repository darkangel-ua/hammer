#include "stdafx.h"
#include <boost/test/auto_unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include "enviroment.h"
#include <hammer/src/engine.h>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

struct instantiation_tests
{
   instantiation_tests() : engine_(test_data_path)
   {

   }
   
   const project& load(const char* test_dir)
   {
      return engine_.load_project(fs::path("instantiation_tests") / test_dir);
   } 

   engine engine_;      
};

BOOST_FIXTURE_TEST_CASE(empty_project, instantiation_tests)
{
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load("empty_project"));
}

BOOST_FIXTURE_TEST_CASE(named_project, instantiation_tests)
{
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load("named_project"));
}

BOOST_FIXTURE_TEST_CASE(lib1_project, instantiation_tests)
{
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load("lib1_project"));
}
