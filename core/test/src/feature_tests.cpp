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
   pool p;
   feature_registry fr(&p);

   vector<string> empty_values;
   vector<string> values;
      values.push_back("on");
      values.push_back("of");

   feature_attributes base_fa = {0};
   feature_def& d1 = fr.add_feature_def("name", empty_values, base_fa);
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
   pool p;
   feature_registry fr(&p);

   feature_def& d = fr.add_feature_def("toolset");
   subfeature_def& sd = d.add_subfeature("version");
   BOOST_REQUIRE_THROW(d.add_subfeature("version"), std::exception);
   BOOST_CHECK(d.find_subfeature(sd.name()) != NULL);
}
