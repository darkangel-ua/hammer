#include "stdafx.h"
#include "enviroment.h"
#include <hammer/src/feature.h>
#include <hammer/src/subfeature.h>
#include <hammer/src/feature_registry.h>
#include <hammer/src/pool.h>

using namespace hammer;
using namespace std;

struct feature_registry_test
{
   feature_registry_test() : registry_(&p_) {}

   pool p_;
   feature_registry registry_;
};

BOOST_FIXTURE_TEST_CASE(create_feature, feature_registry_test)
{
   feature_def toolset_def("toolset");
   toolset_def.extend_legal_values("msvc");
   BOOST_REQUIRE_NO_THROW(registry_.add_def(toolset_def));
   BOOST_REQUIRE_THROW(registry_.add_def(toolset_def), std::exception);
   BOOST_REQUIRE_THROW(registry_.create_feature("svn", ""), std::exception);
   BOOST_REQUIRE_THROW(registry_.create_feature("", ""), std::exception);
};

struct complex_feature_registry_test : public feature_registry_test
{
   complex_feature_registry_test()
   {
      feature_def toolset_def("toolset");
      toolset_def.extend_legal_values("msvc");
      subfeature_def version_def("version");
      version_def.extend_legal_values("8.0");
      toolset_def.add_subfeature(version_def);
      registry_.add_def(toolset_def);
   }
};

BOOST_FIXTURE_TEST_CASE(create_subfeature, complex_feature_registry_test)
{
   feature* toolset = NULL;
   BOOST_REQUIRE_NO_THROW(toolset = registry_.create_feature("toolset", "msvc"));
   BOOST_REQUIRE(toolset != NULL);
   feature* same_toolset = NULL;
   BOOST_REQUIRE_NO_THROW(same_toolset = registry_.create_feature("toolset", "msvc"));
   BOOST_REQUIRE(same_toolset != NULL);
   BOOST_CHECK(toolset == same_toolset);
   BOOST_REQUIRE_NO_THROW(toolset = registry_.create_feature(*toolset, "version", "8.0"));
   const subfeature* version = NULL;
   BOOST_REQUIRE_NO_THROW(version = toolset->find_subfeature("version"));
   BOOST_CHECK_EQUAL(version->name(), "version");
   BOOST_CHECK_EQUAL(version->value(), "8.0");
}

/*
BOOST_FIXTURE_TEST_CASE(parse_feature, complex_feature_registry_test)
{
   feature* toolset_msvc = NULL;
   BOOST_REQUIRE_NO_THROW(toolset_msvc = registry_.create_feature("toolset", "msvc"));
   BOOST_CHECK_EQUAL(toolset_msvc->name(), "toolset");
   BOOST_CHECK_EQUAL(toolset_msvc->value(), "msvc");

   BOOST_REQUIRE_NO_THROW(toolset_msvc = registry_.create_feature("toolset", "msvc-8.0"));
   BOOST_CHECK_EQUAL(toolset_msvc->name(), "toolset");
   BOOST_CHECK_EQUAL(toolset_msvc->value(), "msvc");
}
*/

