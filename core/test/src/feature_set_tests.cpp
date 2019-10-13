#include "stdafx.h"
#include "enviroment.h"
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>

using namespace hammer;
using namespace std;

namespace 
{
   struct enviroment
   {
      enviroment()
      {
         {
            feature_attributes attrs = {0};
            attrs.free = 1;
            fr_.add_feature_def("define", {}, attrs);
         }

         {
            feature_attributes attrs = {0};
            attrs.free = 1;
            fr_.add_feature_def("optimization", {"off", "speed", "size"}, attrs);
         }

         {
            feature_attributes attrs = {0};
            attrs.free = 1;
            fr_.add_feature_def("debug-symbols", {"on", "off"}, attrs);
         }

         {
            feature_attributes attrs = {0};
            attrs.propagated = 1;
            fr_.add_feature_def("link", {"shared", "static"}, attrs);
         }

         {
            feature_attributes attrs = {0};
            attrs.composite = 1;
            attrs.propagated = 1;
            feature_set* debug_components = fr_.make_set();
            debug_components->join("optimization", "off").
                              join("debug-symbols", "on");

            feature_set* release_components = fr_.make_set();
            release_components->join("optimization", "on").
                                join("debug-symbols", "off");

            feature_def& def = fr_.add_feature_def("variant", {"debug", "release"}, attrs);
            def.compose("debug", debug_components);
            def.compose("release", release_components);
         }

      }

      feature_registry fr_;
   }; 
}

BOOST_FIXTURE_TEST_CASE(join_free, enviroment)
{
   feature_set* f = fr_.make_set();
   f->join("define", "NDEBUG=1");
   BOOST_REQUIRE_EQUAL(f->size(), 1);
   f->join("define", "WIN32");
   BOOST_REQUIRE_EQUAL(f->size(), 2);
   feature_set::const_iterator i = f->find("define");
   BOOST_REQUIRE(i != f->end());
   BOOST_REQUIRE((**i).value() == "NDEBUG=1");
   ++i;
   BOOST_REQUIRE((**i).value() == "WIN32");
}

BOOST_FIXTURE_TEST_CASE(join_non_free, enviroment)
{
   feature_set* f = fr_.make_set();
   f->join("link", "shared");
   BOOST_REQUIRE_EQUAL(f->size(), 1);
   f->join("link", "static");
   BOOST_REQUIRE_EQUAL(f->size(), 1);
   feature_set::const_iterator i = f->find("link");
   BOOST_REQUIRE(i != f->end());
   BOOST_REQUIRE(i == f->begin());
}

BOOST_FIXTURE_TEST_CASE(join_non_free_ns, enviroment)
{
   feature_value_ns_ptr cpp_ns = fr_.get_or_create_feature_value_ns("c++");
   feature_value_ns_ptr qt_ns = fr_.get_or_create_feature_value_ns("qt");
   fr_.add_feature_def("toolset", { {"gcc", cpp_ns}, {"msvc", cpp_ns}, {"qt", qt_ns} } );

   feature_set& fs = *fr_.make_set();
   fs.join("toolset", "gcc");
   BOOST_REQUIRE_EQUAL(fs.size(), 1);
   fs.join("toolset", "qt");
   BOOST_REQUIRE_EQUAL(fs.size(), 2);

   vector<feature_ref> toolsets;
   for (auto i = fs.find("toolset"); i != fs.end(); i = fs.find(i + 1, "toolset"))
      toolsets.push_back(**i);

   vector<feature_ref> expected_toolsets =
      { fr_.create_feature("toolset", "gcc"),
        fr_.create_feature("toolset", "qt")
      };

   sort(toolsets.begin(), toolsets.end(), [](feature_ref lhs, feature_ref rhs) { return lhs < rhs; });
   sort(expected_toolsets.begin(), expected_toolsets.end(), [](feature_ref lhs, feature_ref rhs) { return lhs < rhs; });

   BOOST_CHECK(toolsets[0] == expected_toolsets[0]);
   BOOST_CHECK(toolsets[1] == expected_toolsets[1]);
}

BOOST_FIXTURE_TEST_CASE(simple_composite, enviroment)
{
   feature_set* f = fr_.make_set();
   f->join("variant", "debug");
   BOOST_REQUIRE_EQUAL(f->size(), 3);
}
