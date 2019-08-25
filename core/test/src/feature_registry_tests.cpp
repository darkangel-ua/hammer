#include "stdafx.h"
#include "enviroment.h"
#include <hammer/core/feature.h>
#include <hammer/core/subfeature.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_value_ns.h>
#include <hammer/core/feature_set.h>

using namespace hammer;
using namespace std;

struct feature_registry_test
{
   feature_registry registry_;
};

BOOST_FIXTURE_TEST_CASE(feature_value_namespace_construct_1, feature_registry_test)
{
   feature_value_ns_ptr cpp_ns;
   BOOST_REQUIRE_NO_THROW(cpp_ns = registry_.get_or_create_feature_value_ns("c++"));
   BOOST_CHECK_EQUAL(cpp_ns->name(), "c++");

   feature_value_ns_ptr cpp_ns_1;
   BOOST_REQUIRE_NO_THROW(cpp_ns_1 = registry_.get_or_create_feature_value_ns("c++"));
   BOOST_CHECK_EQUAL(cpp_ns, cpp_ns_1);

   feature_value_ns_ptr qt_ns;
   BOOST_REQUIRE_NO_THROW(qt_ns = registry_.get_or_create_feature_value_ns("qt"));
   BOOST_CHECK_EQUAL(qt_ns->name(), "qt");

   feature_value_ns_ptr cpp_ns_2;
   BOOST_REQUIRE_NO_THROW(cpp_ns_2 = registry_.get_or_create_feature_value_ns("c++"));
   BOOST_CHECK_EQUAL(cpp_ns, cpp_ns_2);
}

BOOST_FIXTURE_TEST_CASE(feature_def_1, feature_registry_test)
{
   feature_def& toolset_fd = registry_.add_feature_def("toolset");
   BOOST_CHECK(toolset_fd.get_defaults().empty());
}

BOOST_FIXTURE_TEST_CASE(feature_def_global_ns, feature_registry_test)
{
   feature_def& toolset_fd = registry_.add_feature_def("toolset", { {"gcc"}, {"msvc"} } );
   BOOST_REQUIRE_EQUAL(toolset_fd.get_defaults().size(), 1);
   BOOST_CHECK_EQUAL(toolset_fd.get_defaults().front().value_, "gcc");
   BOOST_CHECK_EQUAL(toolset_fd.get_defaults().front().ns_, feature_value_ns_ptr());
}

BOOST_FIXTURE_TEST_CASE(feature_def_two_ns, feature_registry_test)
{
   feature_value_ns_ptr cpp_ns = registry_.get_or_create_feature_value_ns("c++");
   feature_value_ns_ptr qt_ns = registry_.get_or_create_feature_value_ns("qt");
   feature_def& toolset_fd = registry_.add_feature_def("toolset", { {"gcc", cpp_ns}, {"qt", qt_ns}, {"msvc", cpp_ns} } );
   BOOST_CHECK_EQUAL(toolset_fd.get_defaults().size(), 2);
   BOOST_CHECK(toolset_fd.defaults_contains("gcc"));
   BOOST_CHECK(toolset_fd.defaults_contains("qt"));
}

BOOST_FIXTURE_TEST_CASE(feature_def_extend_with_ns, feature_registry_test)
{
   feature_value_ns_ptr cpp_ns = registry_.get_or_create_feature_value_ns("c++");
   feature_value_ns_ptr qt_ns = registry_.get_or_create_feature_value_ns("qt");
   feature_def& toolset_fd = registry_.add_feature_def("toolset" );

   BOOST_REQUIRE_NO_THROW(toolset_fd.extend_legal_values("gcc", cpp_ns));
   BOOST_CHECK_EQUAL(toolset_fd.get_defaults().size(), 1);

   BOOST_REQUIRE_NO_THROW(toolset_fd.extend_legal_values("qt", qt_ns));
   BOOST_CHECK_EQUAL(toolset_fd.get_defaults().size(), 2);

   BOOST_CHECK(toolset_fd.defaults_contains("gcc"));
   BOOST_CHECK(toolset_fd.defaults_contains("qt"));

   BOOST_REQUIRE_NO_THROW(toolset_fd.extend_legal_values("msvc", cpp_ns));
   BOOST_CHECK_EQUAL(toolset_fd.get_defaults().size(), 2);

   BOOST_CHECK(toolset_fd.defaults_contains("gcc"));
   BOOST_CHECK(toolset_fd.defaults_contains("qt"));
}

BOOST_FIXTURE_TEST_CASE(add_defaults_1, feature_registry_test)
{
   registry_.add_feature_def("toolset", { {"gcc"}, {"msvc"} } );

   feature_set& fs = *registry_.make_set();
   BOOST_REQUIRE(fs.empty());
   BOOST_REQUIRE_NO_THROW(registry_.add_defaults(fs));
   BOOST_REQUIRE_EQUAL(fs.size(), 1);
   auto i = fs.find("toolset");
   BOOST_REQUIRE(i != fs.end());
   feature& f = **i;
   BOOST_CHECK_EQUAL(f.name(), "toolset");
   BOOST_CHECK_EQUAL(f.value(), "gcc");
}

BOOST_FIXTURE_TEST_CASE(add_defaults_2, feature_registry_test)
{
   feature_value_ns_ptr cpp_ns = registry_.get_or_create_feature_value_ns("c++");
   feature_value_ns_ptr qt_ns = registry_.get_or_create_feature_value_ns("qt");
   registry_.add_feature_def("toolset", { {"gcc", cpp_ns}, {"msvc", cpp_ns}, {"qt", qt_ns} } );

   feature_set& fs = *registry_.make_set();
   BOOST_REQUIRE(fs.empty());
   BOOST_REQUIRE_NO_THROW(registry_.add_defaults(fs));
   BOOST_REQUIRE_EQUAL(fs.size(), 2);

   vector<feature_ref> defaults;
   for (auto i = fs.find("toolset"); i != fs.end(); i = fs.find(i + 1, "toolset"))
      defaults.push_back(**i);

   sort(defaults.begin(), defaults.end(), [](feature_ref lhs, feature_ref rhs) { return lhs < rhs; });
   BOOST_REQUIRE_EQUAL(defaults.size(), 2);

   const vector<feature_ref> expected_defaults =
      { registry_.create_feature("toolset", "gcc"),
        registry_.create_feature("toolset", "qt")
      };

   BOOST_CHECK(defaults[0] == expected_defaults[0]);
   BOOST_CHECK(defaults[1] == expected_defaults[1]);
}

BOOST_FIXTURE_TEST_CASE(create_feature, feature_registry_test)
{
   feature_def& toolset_def = registry_.add_feature_def("toolset");
   toolset_def.extend_legal_values("msvc", {});
   BOOST_REQUIRE_THROW(registry_.add_feature_def("toolset");, std::exception);
   BOOST_REQUIRE_THROW(registry_.create_feature("", ""), std::exception);
};

struct complex_feature_registry_test : public feature_registry_test
{
   complex_feature_registry_test()
   {
      feature_def& toolset_def = registry_.add_feature_def("toolset");
      toolset_def.extend_legal_values("msvc", {});
      subfeature_def& version_def = toolset_def.add_subfeature("version");
      version_def.extend_legal_values("msvc", "8.0");
   }
};

BOOST_FIXTURE_TEST_CASE(create_subfeature, complex_feature_registry_test)
{
   const feature* toolset = nullptr;
   BOOST_REQUIRE_NO_THROW(toolset = &registry_.create_feature("toolset", "msvc").get());
   BOOST_REQUIRE(toolset);
   const feature* same_toolset = nullptr;
   BOOST_REQUIRE_NO_THROW(same_toolset = &registry_.create_feature("toolset", "msvc").get());
   BOOST_REQUIRE(same_toolset);
   BOOST_CHECK(toolset == same_toolset);
   BOOST_REQUIRE_NO_THROW(toolset = &registry_.create_feature(*toolset, "version", "8.0").get());
   const subfeature* version = nullptr;
   BOOST_REQUIRE_NO_THROW(version = toolset->find_subfeature("version"));
   BOOST_CHECK_EQUAL(version->name(), "version");
   BOOST_CHECK_EQUAL(version->value(), "8.0");
}

BOOST_FIXTURE_TEST_CASE(parse_feature, complex_feature_registry_test)
{
   const feature* toolset_msvc = nullptr;
   BOOST_REQUIRE_NO_THROW(toolset_msvc = &registry_.create_feature("toolset", "msvc").get());
   BOOST_CHECK_EQUAL(toolset_msvc->name(), "toolset");
   BOOST_CHECK_EQUAL(toolset_msvc->value(), "msvc");

   BOOST_REQUIRE_NO_THROW(toolset_msvc = &registry_.create_feature("toolset", "msvc-8.0").get());
   BOOST_CHECK_EQUAL(toolset_msvc->name(), "toolset");
   BOOST_CHECK_EQUAL(toolset_msvc->value(), "msvc");
}
