#include "stdafx.h"
#include "enviroment.h"
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/requirements_decl.h>

using namespace hammer;
using namespace std;

BOOST_AUTO_TEST_SUITE(requirements_decl_tests)

struct environment
{
   environment()
   {
      {
         feature_attributes attrs = {0};
         attrs.propagated = 1;
         fr_.add_feature_def("link", { "shared", "static" }, attrs);
      }

      {
         feature_attributes attrs = {0};
         attrs.free = 1;
         fr_.add_feature_def("define", {}, attrs);
      }

      {
         feature_attributes attrs = {0};
         attrs.propagated = 1;
         feature_def& toolset_def = fr_.add_feature_def("toolset", { "gcc", "msvc" }, attrs);
         subfeature_def& toolset_version_def = toolset_def.add_subfeature("version");
         toolset_version_def.extend_legal_values("gcc", "4");
         toolset_version_def.extend_legal_values("gcc", "5");
      }
   }

   feature_registry fr_;
};

requirements_decl
make_requirements_decl(feature_registry& fr,
                       const string& toolset)
{
   requirements_decl requirements;

   auto_ptr<linear_and_condition> condition(new linear_and_condition);
   condition->add(fr.create_feature("toolset", toolset));
   condition->result(fr.create_feature("link", "shared"));
   requirements.add(auto_ptr<requirement_base>(condition));

   return requirements;
}

BOOST_FIXTURE_TEST_CASE(matched_condition, environment)
{
   // build request: <toolset>gcc
   // requirements:  <toolset>gcc:<link>shared
   // result:        <link>shared

   feature_set& build_request = *fr_.make_set();
   build_request.join("toolset", "gcc");

   requirements_decl requirements = make_requirements_decl(fr_, "gcc");
   feature_set& evaluation_result = *fr_.make_set();
   requirements.eval(build_request, &evaluation_result, nullptr);

   BOOST_REQUIRE_EQUAL(evaluation_result.size(), 1);
   const feature& link_feature = **evaluation_result.begin();
   BOOST_CHECK_EQUAL(link_feature.name(), "link");
   BOOST_CHECK_EQUAL(link_feature.value(), "shared");
}

BOOST_FIXTURE_TEST_CASE(matched_condition_with_subfeature_1, environment)
{
   // build request: <toolset>gcc
   // requirements:  <toolset>gcc-4:<link>shared
   // result:        <!empty>

   feature_set& build_request = *fr_.make_set();
   build_request.join("toolset", "gcc");

   requirements_decl requirements = make_requirements_decl(fr_, "gcc-4");
   feature_set& evaluation_result = *fr_.make_set();
   requirements.eval(build_request, &evaluation_result, nullptr);

   BOOST_REQUIRE_EQUAL(evaluation_result.size(), 0);
}

BOOST_FIXTURE_TEST_CASE(matched_condition_with_subfeature_2, environment)
{
   // build request: <toolset>gcc-4
   // requirements:  <toolset>gcc:<link>shared
   // result:        <link>shared

   feature_set& build_request = *fr_.make_set();
   build_request.join("toolset", "gcc-4");

   requirements_decl requirements = make_requirements_decl(fr_, "gcc");
   feature_set& evaluation_result = *fr_.make_set();
   requirements.eval(build_request, &evaluation_result, nullptr);

   BOOST_REQUIRE_EQUAL(evaluation_result.size(), 1);
   const feature& link_feature = **evaluation_result.begin();
   BOOST_CHECK_EQUAL(link_feature.name(), "link");
   BOOST_CHECK_EQUAL(link_feature.value(), "shared");
}

BOOST_FIXTURE_TEST_CASE(matched_condition_with_subfeature_3, environment)
{
   // build request: <toolset>gcc-4
   // requirements:  <toolset>gcc-4:<link>shared
   // result:        <link>shared

   feature_set& build_request = *fr_.make_set();
   build_request.join("toolset", "gcc-4");

   requirements_decl requirements = make_requirements_decl(fr_, "gcc-4");
   feature_set& evaluation_result = *fr_.make_set();
   requirements.eval(build_request, &evaluation_result, nullptr);

   BOOST_REQUIRE_EQUAL(evaluation_result.size(), 1);
   const feature& link_feature = **evaluation_result.begin();
   BOOST_CHECK_EQUAL(link_feature.name(), "link");
   BOOST_CHECK_EQUAL(link_feature.value(), "shared");
}

BOOST_FIXTURE_TEST_CASE(not_matched_condition_with_subfeature_4, environment)
{
   // build request: <toolset>gcc-4
   // requirements:  <toolset>gcc-5:<link>shared
   // result:        <!empty>

   feature_set& build_request = *fr_.make_set();
   build_request.join("toolset", "gcc-4");

   requirements_decl requirements = make_requirements_decl(fr_, "gcc-5");
   feature_set& evaluation_result = *fr_.make_set();
   requirements.eval(build_request, &evaluation_result, nullptr);

   BOOST_REQUIRE_EQUAL(evaluation_result.size(), 0);
}

BOOST_FIXTURE_TEST_CASE(not_matched_conditions, environment)
{
   // build request: <toolset>msvc
   // requirements:  <toolset>gcc:<link>shared
   // result:        <!empty>

   feature_set& build_request = *fr_.make_set();
   build_request.join("toolset", "msvc");

   requirements_decl requirements = make_requirements_decl(fr_, "gcc");

   feature_set& evaluation_result = *fr_.make_set();
   requirements.eval(build_request, &evaluation_result, nullptr);

   BOOST_REQUIRE_EQUAL(evaluation_result.size(), 0);
}

BOOST_FIXTURE_TEST_CASE(conditions_match_defaults, environment)
{
   // build request: <toolset>gcc
   // requirements:  <link>shared:<define>FOO
   // result:        <define>FOO

   feature_set& build_request = *fr_.make_set();
   build_request.join("toolset", "gcc");

   requirements_decl requirements;

   auto_ptr<linear_and_condition> condition(new linear_and_condition);
   condition->add(fr_.create_feature("link", "shared"));
   condition->result(fr_.create_feature("define", "FOO"));
   requirements.add(auto_ptr<requirement_base>(condition));

   feature_set& evaluation_result = *fr_.make_set();
   requirements.eval(build_request, &evaluation_result, nullptr);

   BOOST_REQUIRE_EQUAL(evaluation_result.size(), 1);
   const feature& define_feature = **evaluation_result.begin();
   BOOST_CHECK_EQUAL(define_feature.name(), "define");
   BOOST_CHECK_EQUAL(define_feature.value(), "FOO");
}

BOOST_AUTO_TEST_SUITE_END()
