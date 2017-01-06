#include "stdafx.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/bind.hpp>
#include "enviroment.h"
#include <hammer/core/engine.h>
#include "jcf_parser.h"
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <iostream>
#include "options.h"

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;
using namespace boost::unit_test;

struct instantiation_tests : public setuped_engine
{
   instantiation_tests()
   {
   }
   
   void run_test(const fs::path& test_data_path)
   {
      options opts(test_data_path / "hamfile");
      if (opts.exists("skip"))
         return;

      project& p = engine_.load_project(test_data_path);

      // if instantiation.jcf presents that run checker 
      // FIXME: else we just parse project for now
      if (opts.exists("should-fail") || exists(test_data_path / "instantiation.jcf")) {
         feature_set* build_request = engine_.feature_registry().make_set();

         vector<basic_target*> instantiated_targets;
         if (opts.exists("should-fail"))
            BOOST_CHECK_THROW(p.instantiate("test", *build_request, &instantiated_targets), std::exception)
         else {
            p.instantiate("test", *build_request, &instantiated_targets);
            BOOST_CHECK(checker_.parse(test_data_path / "instantiation.jcf"));
            checker_.walk(instantiated_targets, &engine_);
         }
      }
   }

   jcf_parser checker_;
   string name_;
};

static
void test_function(const fs::path& test_data_path)
{
   instantiation_tests tests;
   tests.run_test(test_data_path);
}

void init_instantiation_tests(const fs::path& test_data_path)
{
   test_suite* ts = BOOST_TEST_SUITE("instantiation");
   for(fs::directory_iterator i(test_data_path / "instantiation_tests"); i != fs::directory_iterator(); ++i)
      ts->add(make_test_case(boost::bind(&test_function, i->path()), i->path().filename().string()));

   framework::master_test_suite().add(ts);
}
             
