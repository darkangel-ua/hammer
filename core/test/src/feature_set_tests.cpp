#include "stdafx.h"
#include "enviroment.h"
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <boost/assign/list_of.hpp>

using namespace hammer;
using namespace std;
using namespace boost::assign;

namespace 
{
   struct enviroment
   {
      enviroment()
      {
         {
            feature_attributes attrs = {0};
            attrs.free = 1;
            fr_.add_feature_def("define", vector<string>(), attrs);
         }

         {
            feature_attributes attrs = {0};
            attrs.free = 1;
            fr_.add_feature_def("optimization", list_of<string>("off")("speed")("size"), attrs);
         }

         {
            feature_attributes attrs = {0};
            attrs.free = 1;
            fr_.add_feature_def("debug-symbols", list_of<string>("on")("off"), attrs);
         }

         {
            feature_attributes attrs = {0};
            attrs.propagated = 1;
            fr_.add_feature_def("link", list_of<string>("shared")("static"), attrs);
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

            feature_def& def = fr_.add_feature_def("variant", list_of<string>("debug")("release"), attrs);
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

BOOST_FIXTURE_TEST_CASE(simple_composite, enviroment)
{
   feature_set* f = fr_.make_set();
   f->join("variant", "debug");
   BOOST_REQUIRE_EQUAL(f->size(), 3);
}
