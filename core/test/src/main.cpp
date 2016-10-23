#include "stdafx.h"
#include <stdexcept>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

boost::filesystem::path test_data_path;

namespace fs = boost::filesystem;

void init_instantiation_tests(const fs::path& test_data_root);
void init_generators_tests(const fs::path& test_data_path);

boost::unit_test::test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
   if (argc < 2)
      throw std::runtime_error("Need test data directory path");
       
   
   test_data_path = fs::path(argv[1]); 
   if (!test_data_path.has_root_path()) {
      test_data_path = fs::current_path() / test_data_path;
      test_data_path.normalize();
   }

   init_instantiation_tests(test_data_path);
//   init_generators_tests(test_data_path);

   return 0;            
}                             
