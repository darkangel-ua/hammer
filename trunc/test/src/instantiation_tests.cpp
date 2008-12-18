#include "stdafx.h"
#include <boost/test/auto_unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include "enviroment.h"
#include <hammer/src/engine.h>
#include "jcf_parser.h"
#include <hammer/src/feature_registry.h>
#include <hammer/src/feature_set.h>
#include <iostream>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

struct instantiation_tests : public setuped_engine
{
   instantiation_tests()
   {
   }
   
   const project& load()
   {
      return engine_.load_project(test_data_path / "instantiation_tests" / name_);
   } 
   
   void check(const vector<basic_target*> targets)
   {
      std::cout << "check '" << name_ << "'\n";
      BOOST_CHECK(checker_.parse(test_data_path / "instantiation_tests" / name_ / "instantiation.jcf")); 
      BOOST_CHECK(checker_.walk(targets, &engine_));
   }

   jcf_parser checker_;
   string name_;
};

/*
BOOST_FIXTURE_TEST_CASE(empty_project, instantiation_tests)
{
   name_ = "empty_project";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   BOOST_CHECK_EQUAL(p->name(), pstring());
   BOOST_CHECK(p->targets().empty());
}

BOOST_FIXTURE_TEST_CASE(named_project, instantiation_tests)
{
   name_ = "named_project";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   BOOST_REQUIRE_EQUAL(p->name(), "test");
   BOOST_CHECK(p->targets().empty());
}

BOOST_FIXTURE_TEST_CASE(lib1_project, instantiation_tests)
{
   name_ = "lib1_project";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
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
   build_request->join("variant", "debug");
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
   build_request->join("variant", "debug");
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
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   // по идее должен кинуть ексепшин что два usage requirements имеют конфликтные базовые свойства
   BOOST_REQUIRE_THROW(p->instantiate("test", *build_request, &tt), std::exception);
}

BOOST_FIXTURE_TEST_CASE(three_projects, instantiation_tests)
{
   name_ = "three_projects";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(simple_project, instantiation_tests)
{
   name_ = "simple_project";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(jamroot_test, instantiation_tests)
{
   name_ = "jamroot_test";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(glob, instantiation_tests)
{
   name_ = "glob";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(explicit_, instantiation_tests)
{
   name_ = "explicit";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(feature_source_and_library, instantiation_tests)
{
   name_ = "feature_source_and_library";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(alternatives, instantiation_tests)
{
   name_ = "alternatives";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(conditional_requirements, instantiation_tests)
{
   name_ = "conditional_requirements";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(alias, instantiation_tests)
{
   name_ = "alias";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(use_project, instantiation_tests)
{
   name_ = "use_project";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(feature_use, instantiation_tests)
{
   name_ = "feature_use";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(use_project_multi, instantiation_tests)
{
   name_ = "use_project_multi";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(alternatives_override, instantiation_tests)
{
   name_ = "alternatives_override";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}

BOOST_FIXTURE_TEST_CASE(no_alternatives_1, instantiation_tests)
{
   name_ = "no_alternatives_1";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   BOOST_REQUIRE_THROW(p->instantiate("test", *build_request, &tt), std::exception);
}

BOOST_FIXTURE_TEST_CASE(header_lib, instantiation_tests)
{
   name_ = "header_lib";
   const project* p = 0;
   BOOST_REQUIRE_NO_THROW(p = &load());
   BOOST_REQUIRE(p);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("variant", "debug");
   vector<basic_target*> tt;
   p->instantiate("test", *build_request, &tt);
   BOOST_REQUIRE_EQUAL(tt.size(), size_t(1));
   check(tt);
}
*/
