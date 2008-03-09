#include "stdafx.h"
#include <boost/test/auto_unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/bind.hpp>
#include <stdexcept>
#include <hammer/src/call_resolver.h>
#include <hammer/src/engine.h>
#include <hammer/src/parser.h>
#include <hammer/src/hammer_walker_context.h>

static boost::filesystem::path test_data_path;

using namespace std;
namespace fs = boost::filesystem;
using namespace hammer;

boost::unit_test::test_suite*            
init_unit_test_suite( int argc, char* argv[] )
{
   if (argc < 2)
      throw runtime_error("Need test data directory path");
       
   test_data_path = boost::filesystem::path(argv[1]); 

   return 0;            
}                             

struct test_evn                   
{
   test_evn::test_evn()
   {
   }
};
     
BOOST_FIXTURE_TEST_CASE(parser_test, test_evn)
{       
//   _CrtSetBreakAlloc(1240);
   parser p;
   fs::path tp = test_data_path / "parser_test";
   BOOST_CHECK(p.parse(tp / "empty"));
   BOOST_CHECK(p.parse(tp / "rule_no_args"));
   BOOST_CHECK(p.parse(tp / "comments"));
   BOOST_CHECK(p.parse(tp / "rule_with_empty_args"));
   BOOST_CHECK(p.parse(tp / "rule_with_string_list"));
   BOOST_CHECK(p.parse(tp / "rule_with_feature_list"));
}

static bool no_args_test_pass = false;

static void no_args()
{
   no_args_test_pass = true;   
}

struct call_resolver_env
{
   call_resolver_env() : tp_(test_data_path / "call_resolver_test"), engine_(tp_)
   {
      hwc_.call_resolver_ = &resolver_;
      hwc_.engine_ = &engine_;
   }
   
   bool parse(const char* file_name) { return parser_.parse(tp_ / file_name); }
   void walk() { parser_.walk(&hwc_); } 

   fs::path tp_;
   parser parser_;
   engine engine_;
   call_resolver resolver_;
   hammer_walker_context hwc_;
};

BOOST_FIXTURE_TEST_CASE(no_args_test, call_resolver_env)
{       
   resolver_.insert("no_args", boost::function<void()>(&no_args));
   BOOST_CHECK(parse("no_args"));
   walk(); 
   BOOST_CHECK(no_args_test_pass);
}

BOOST_FIXTURE_TEST_CASE(non_existing_rule_test, call_resolver_env)
{       
   BOOST_CHECK(parse("non_existing_rule"));
   BOOST_REQUIRE_THROW(walk(), std::exception); 
}

static void one_arg_rule(std::vector<pstring>* a)
{
   BOOST_REQUIRE(a);
   BOOST_REQUIRE_EQUAL(a->size(), size_t(1));
   BOOST_CHECK_EQUAL(a->at(0), "arg1");
}

BOOST_FIXTURE_TEST_CASE(one_arg_rule_test, call_resolver_env)
{       
   resolver_.insert("one_arg", boost::function<void (vector<pstring>*)>(&one_arg_rule));
   BOOST_CHECK(parse("one_arg"));
   BOOST_REQUIRE_NO_THROW(walk()); 
}

static void one_optional_arg(std::vector<pstring>* a, bool must_be_null)
{
   if (must_be_null)
   {
      BOOST_REQUIRE(a == 0);
   }
   else
   {
      BOOST_REQUIRE(a);
      BOOST_REQUIRE_EQUAL(a->size(), size_t(1));
      BOOST_CHECK_EQUAL(a->at(0), "arg1");
   }
}

BOOST_FIXTURE_TEST_CASE(one_optional_arg_rule, call_resolver_env)
{       
   resolver_.insert("one_optional_arg_with_arg", boost::function<void (vector<pstring>*)>(boost::bind(&one_optional_arg, _1, false)));
   resolver_.insert("one_optional_arg_without_arg", boost::function<void (vector<pstring>*)>(boost::bind(&one_optional_arg, _1, true)));
   BOOST_CHECK(parse("one_optional_arg"));
   BOOST_REQUIRE_NO_THROW(walk()); 
}

static void one_required_arg(std::vector<pstring>& a)
{
   BOOST_REQUIRE_EQUAL(a.size(), size_t(1));
   BOOST_REQUIRE_EQUAL(a[0], "arg1");
}

BOOST_FIXTURE_TEST_CASE(one_required_arg_with_arg_rule, call_resolver_env)
{       
   resolver_.insert("one_required_arg", boost::function<void (vector<pstring>&)>(&one_required_arg));
   BOOST_CHECK(parse("one_required_arg_with_arg"));
   BOOST_REQUIRE_NO_THROW(walk()); 
}

BOOST_FIXTURE_TEST_CASE(one_required_arg_without_arg_rule, call_resolver_env)
{       
   resolver_.insert("one_required_arg", boost::function<void (vector<pstring>&)>(&one_required_arg));
   BOOST_CHECK(parse("one_required_arg_without_arg"));
   BOOST_REQUIRE_THROW(walk(), std::exception); 
}