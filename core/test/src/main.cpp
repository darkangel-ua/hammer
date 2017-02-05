#include "stdafx.h"
#include <stdexcept>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/debug.hpp>
#include "enviroment.h"

boost::filesystem::path test_data_path;

using namespace std;
namespace fs = boost::filesystem;

void init_instantiation_tests(const fs::path& test_data_root);
void init_generators_tests(const fs::path& test_data_path);
void init_warehouse_auto_tests(const fs::path& test_data_path);

static
void complete_build_test_function(const fs::path& working_directory)
{
   complete_build_tests_environment env;

   env.run_test(working_directory);
}

boost::unit_test::test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
   if (argc < 2)
      throw std::runtime_error("Need test data directory path");
       
   boost::debug::detect_memory_leaks(false);

   test_data_path = fs::path(argv[1]); 
   if (!test_data_path.has_root_path()) {
      test_data_path = fs::current_path() / test_data_path;
      test_data_path.normalize();
   }

   init_instantiation_tests(test_data_path);
//   init_generators_tests(test_data_path);
   init_warehouse_auto_tests(test_data_path);

   add_tests_from_filesystem(test_data_path / "htmpl", "htmpl", bind(complete_build_test_function, placeholders::_1));
   add_tests_from_filesystem(test_data_path / "qt", "qt", bind(complete_build_test_function, placeholders::_1));

   return 0;            
}                             
