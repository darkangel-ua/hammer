#include "stdafx.h"
#include <boost/test/auto_unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include "enviroment.h"
#include <hammer/src/engine.h>
#include "jcf_parser.h"
#include <hammer/src/feature_registry.h>
#include <hammer/src/feature_set.h>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

struct instantiation_tests
{
   instantiation_tests() : engine_(test_data_path)
   {

   }
   
   const project& load()
   {
      return engine_.load_project(fs::path("instantiation_tests") / name_);
   } 
   
   void check(const vector<basic_target*> targets)
   {
      BOOST_CHECK(checker_.parse(test_data_path / "instantiation_tests" / name_ / "instantiation.jcf")); 
      BOOST_CHECK(checker_.walk(targets, &engine_));
   }

   engine engine_;
   jcf_parser checker_;
   string name_;
};

BOOST_FIXTURE_TEST_CASE(empty_project, instantiation_tests)
{
   name_ = "empty_project";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   BOOST_CHECK_EQUAL(p->id(), pstring());
   BOOST_CHECK(p->targets().empty());
}

BOOST_FIXTURE_TEST_CASE(named_project, instantiation_tests)
{
   name_ = "named_project";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   BOOST_REQUIRE_EQUAL(p->id(), "test");
   BOOST_CHECK(p->targets().empty());
}

BOOST_FIXTURE_TEST_CASE(lib1_project, instantiation_tests)
{
   name_ = "lib1_project";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->insert("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("a", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(propagated_features, instantiation_tests)
{
   name_ = "propagated_features";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->insert("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(simple_usage_requirements, instantiation_tests)
{
   name_ = "simple_usage_requirements";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->insert("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(conflicting_usage_requirements, instantiation_tests)
{
   name_ = "conflicting_usage_requirements";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->insert("variant", "debug");
   vector<basic_target*> tt;
   // по идее должен кинуть ексепшин что два usage requirements имеют конфликтные базовые свойства
   BOOST_REQUIRE_THROW(p->instantiate("test", *build_request, &tt), std::exception);
}
