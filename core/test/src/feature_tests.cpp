#include "stdafx.h"
#include "enviroment.h"
#include <hammer/core/feature.h>
#include <hammer/core/feature_def.h>
#include <hammer/core/subfeature_def.h>
#include <hammer/core/subfeature.h>
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

   feature_def& toolset_def = fr.add_feature_def("toolset");
   subfeature_def& toolset_version_def = toolset_def.add_subfeature("version");
   BOOST_REQUIRE_THROW(toolset_def.add_subfeature("version"), std::exception);
   BOOST_CHECK(toolset_def.find_subfeature(toolset_version_def.name()) != NULL);
}

BOOST_AUTO_TEST_CASE(feature_compare_with_subfeatures)
{
   pool p;
   feature_registry fr(&p);

   feature_def& toolset_def = fr.add_feature_def("toolset");
   subfeature_def& toolset_version_def = toolset_def.add_subfeature("version");

   toolset_def.extend_legal_values("msvc");
   toolset_version_def.extend_legal_values("msvc", "11.0");
   toolset_version_def.extend_legal_values("msvc", "12.0");

   const feature& toolset_msvc = *fr.create_feature("toolset", "msvc");

   const feature& toolset_msvc_11 = *fr.create_feature("toolset", "msvc-11.0");
   BOOST_REQUIRE_EQUAL(toolset_msvc_11.subfeatures().size(), 1);
   BOOST_REQUIRE_EQUAL(toolset_msvc_11.subfeatures().front()->value(), "11.0");

   const feature& toolset_msvc_12 = *fr.create_feature("toolset", "msvc-12.0");
   BOOST_REQUIRE_EQUAL(toolset_msvc_12.subfeatures().size(), 1);
   BOOST_REQUIRE_EQUAL(toolset_msvc_12.subfeatures().front()->value(), "12.0");

   BOOST_CHECK(toolset_msvc != toolset_msvc_12);
   BOOST_CHECK(toolset_msvc_11 != toolset_msvc_12);
   BOOST_CHECK(toolset_msvc_11 == toolset_msvc_11);
}
