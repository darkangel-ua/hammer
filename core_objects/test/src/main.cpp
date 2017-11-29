#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include <hammer/core/rule_manager.h>
#include <hammer/core/diagnostic.h>

using namespace std; 
using namespace hammer;
using namespace hammer::parscore;
using namespace boost::unit_test;

class test_feature_set;
class test_sources;

namespace hammer {
   template<>
   struct rule_argument_type_info<test_sources> { static const rule_argument_type ast_type = rule_argument_type::sources; };
   template<>
   struct rule_argument_type_info<test_feature_set> { static const rule_argument_type ast_type = rule_argument_type::feature_set; };

   class project{};
}

static
void lib_rule(invocation_context& ctx,
              const identifier& id)
{

}

BOOST_AUTO_TEST_CASE(lib_rule_test)
{
   const identifier rule_id("lib");
   vector<identifier> rule_arg_names = { "id" };

   rule_manager m;
   m.add_target(rule_id, lib_rule, rule_arg_names);
   
   rule_manager::const_iterator i = m.find(rule_id);
   BOOST_REQUIRE(i != m.end());
   BOOST_REQUIRE_THROW(m.add_target(rule_id, lib_rule, rule_arg_names), std::exception);

   const rule_declaration& rd = i->second;
   BOOST_CHECK_EQUAL(rd.name(), rule_id);
   BOOST_CHECK(rd.result().type() == rule_argument_type::void_);
   
   BOOST_REQUIRE_EQUAL(rd.arguments().size(), 2);
   BOOST_CHECK(rd.arguments()[1].type() == rule_argument_type::identifier);
   BOOST_CHECK_EQUAL(rd.arguments()[1].name(), rule_arg_names[0]);
   BOOST_CHECK_EQUAL(rd.arguments()[1].is_optional(), false);
}

static
void exe_rule(invocation_context& ctx,
              const identifier& id,
              const test_sources& sources,
              const test_feature_set* requirements)
{

}

BOOST_AUTO_TEST_CASE(exe_rule_test)
{  
   const identifier rule_id("exe");
   vector<identifier> rule_arg_names = { "id", "sources", "requirements" };

   rule_manager m;
   m.add_target(rule_id, exe_rule, rule_arg_names);

   rule_manager::const_iterator i = m.find(rule_id);
   BOOST_REQUIRE(i != m.end());
   BOOST_REQUIRE_THROW(m.add_target(rule_id, exe_rule, rule_arg_names), std::exception);

   const rule_declaration& rd = i->second;
   BOOST_CHECK_EQUAL(rd.name(), rule_id);
   BOOST_CHECK(rd.result().type() == rule_argument_type::void_);

   BOOST_REQUIRE_EQUAL(rd.arguments().size(), 4);
   
   BOOST_CHECK(rd.arguments()[1].type() == rule_argument_type::identifier);
   BOOST_CHECK_EQUAL(rd.arguments()[1].name(), rule_arg_names[0]);
   BOOST_CHECK_EQUAL(rd.arguments()[1].is_optional(), false);

   BOOST_CHECK(rd.arguments()[2].type() == rule_argument_type::sources);
   BOOST_CHECK_EQUAL(rd.arguments()[2].name(), rule_arg_names[1]);
   BOOST_CHECK_EQUAL(rd.arguments()[2].is_optional(), false);

   BOOST_CHECK(rd.arguments()[3].type() == rule_argument_type::feature_set);
   BOOST_CHECK_EQUAL(rd.arguments()[3].name(), rule_arg_names[2]);
   BOOST_CHECK_EQUAL(rd.arguments()[3].is_optional(), true);
}

static bool invoke_function_1_invoked = false;

static
void invoke_function_1(invocation_context& ctx,
                       const identifier& id_1,
                       const identifier* id_2,
                       const identifier* id_3)
{
   invoke_function_1_invoked = true;
   BOOST_CHECK_EQUAL(id_1.to_string(), "foo");
   BOOST_REQUIRE(id_2 == nullptr);
   BOOST_REQUIRE(id_3 != nullptr);
   BOOST_CHECK_EQUAL(id_3->to_string(), "bar");
}

BOOST_AUTO_TEST_CASE(invoke_test_1)
{
   const identifier rule_id("invoke_function_1");
   vector<identifier> rule_arg_names = { "id_1", "id_2", "id_3" };

   rule_manager m;
   m.add_target(rule_id, invoke_function_1, rule_arg_names);
   const rule_declaration& rd = m.find(rule_id)->second;

   hammer::project p;
   ostringstream s;
   streamed_diagnostic diag("invoke_test_1", s);
   invocation_context ctx = { p, diag, m };
   identifier id_1("foo");
   identifier* id_2 = nullptr;
   identifier id_3("bar");


   rule_manager_arg_ptr arg_0(new rule_manager_arg<invocation_context>(ctx));
   rule_manager_arg_ptr arg_1(new rule_manager_arg<identifier>(id_1));
   rule_manager_arg_ptr arg_2(new rule_manager_arg<identifier>(id_2));
   rule_manager_arg_ptr arg_3(new rule_manager_arg<identifier>(id_3));

   rule_manager_arguments_t args;
   args.push_back(move(arg_0));
   args.push_back(move(arg_1));
   args.push_back(move(arg_2));
   args.push_back(move(arg_3));

   BOOST_REQUIRE_NO_THROW(rd.invoke(args));

   BOOST_CHECK_EQUAL(invoke_function_1_invoked, true);
}

static bool invoke_function_2_invoked = false;

static
std::unique_ptr<identifier>
invoke_function_2(invocation_context& ctx,
                  const identifier& id_1)
{
   invoke_function_2_invoked = true;
   return std::unique_ptr<identifier>(new identifier("123qwe"));
}

BOOST_AUTO_TEST_CASE(invoke_test_2)
{
   const identifier rule_id("invoke_function_2");
   vector<identifier> rule_arg_names = { "id_1" };

   rule_manager m;
   m.add_target(rule_id, invoke_function_2, rule_arg_names);
   const rule_declaration& rd = m.find(rule_id)->second;

   hammer::project p;
   ostringstream s;
   streamed_diagnostic diag("invoke_test_1", s);
   invocation_context ctx = { p, diag, m };
   identifier id_1("foo");

   rule_manager_arg_ptr arg_0(new rule_manager_arg<invocation_context>(ctx));
   rule_manager_arg_ptr arg_1(new rule_manager_arg<identifier>(id_1));

   rule_manager_arguments_t args;
   args.push_back(move(arg_0));
   args.push_back(move(arg_1));

   rule_manager_arg_ptr result;
   BOOST_REQUIRE_NO_THROW(result = rd.invoke(args));

   BOOST_CHECK_EQUAL(invoke_function_2_invoked, true);
   BOOST_CHECK_EQUAL(*static_cast<identifier*>(result->v_), identifier("123qwe"));
}
