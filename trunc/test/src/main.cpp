#include "stdafx.h"
#include <boost/test/auto_unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <stdexcept>
#include <hammer/src/engine.h>
#include <hammer/src/parser.h>

static boost::filesystem::path test_data_path;

using namespace std;
namespace fs = boost::filesystem;
using namespace hammer;

boost::unit_test::test_suite*            
init_unit_test_suite( int argc, char* argv[] )
{
   if (argc < 2)
      throw runtime_error("Need test data directory path");
       
   test_data_path = boost::filesystem::path(argv[1]); 

   return 0;            
}                             

struct test_evn                   
{
   test_evn::test_evn()
   {
   }
};
     
BOOST_FIXTURE_TEST_CASE(parser_test, test_evn)
{
   parser p;
   BOOST_REQUIRE(p.parse((test_data_path / "parser_test" / "Jamroot").string().c_str()));
}

BOOST_FIXTURE_TEST_CASE(unnamed_project, test_evn)
{
//   engine e(find_root(test_data_path / "unnamed_project"));
//   const project& p = e.load_project("unnamed_project");
}
