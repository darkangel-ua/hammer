#include "stdafx.h"
#include "enviroment.h"
#include <hammer/core/feature.h>
#include <hammer/core/feature_def.h>
#include <hammer/core/subfeature_def.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/pool.h>

using namespace std;
using namespace hammer;

BOOST_AUTO_TEST_CASE(feature_def_simple_methods)
{
   vector<string> empty_values;
   vector<string> values;
      values.push_back("on");
      values.push_back("of");

   feature_attributes base_fa = {0};
   feature_def d1("name", empty_values, base_fa);
   BOOST_CHECK_EQUAL(d1.name(), "name");
   BOOST_CHECK_EQUAL(d1.get_default(), "");
   BOOST_CHECK_THROW(d1.set_default("on"), std::exception);
   d1.extend_legal_values("on");
   BOOST_REQUIRE_EQUAL(d1.legal_values().size(), 1);
   BOOST_REQUIRE_EQUAL(d1.get_default(), "on");
   BOOST_REQUIRE_THROW(d1.extend_legal_values("on"), std::exception);
   BOOST_REQUIRE_NO_THROW(d1.extend_legal_values("off"));
   BOOST_REQUIRE_EQUAL(d1.legal_values().size(), 2);
   BOOST_REQUIRE_EQUAL(d1.get_default(), "on");
}

BOOST_AUTO_TEST_CASE(feature_def_subfeatures)
{
   feature_def d("toolset");
   subfeature_def sd("version");
   BOOST_REQUIRE_NO_THROW(d.add_subfeature(sd));
   BOOST_REQUIRE_THROW(d.add_subfeature(sd), std::exception);
   BOOST_CHECK(d.find_subfeature(sd.name()) != NULL);
}

/*
struct feature_test
{
   feature_test() : registry_(&p_) 
   {
      feature_def toolset_def("toolset");
      toolset_def.extend_legal_values("msvc");
      subfeature_def version_def("version");
      version_def.extend_legal_values("8.0");
      toolset_def.add_subfeature(version_def);
      registry_.add_def(toolset_def);
   }

   pool p_;
   feature_registry registry_;
};
*/

