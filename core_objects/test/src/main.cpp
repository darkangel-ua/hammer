#include "stdafx.h"
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include <hammer/core/rule_manager.h>
#include <boost/bind.hpp>
#include <boost/assign/std/vector.hpp>

using namespace std; 
using namespace hammer;
using namespace hammer::parscore;
using namespace boost::unit_test;
using namespace boost::assign;

static void lib_rule(const identifier& id)
{

}

BOOST_AUTO_TEST_CASE(lib_rule_test)
{
   const identifier rule_id("lib");
   vector<identifier> rule_arg_names;
   rule_arg_names += "id";

   rule_manager m;
   m.add_target(rule_id, 
                boost::function<void(const identifier&)>(lib_rule), 
                rule_arg_names);
   
   rule_manager::const_iterator i = m.find(rule_id);
   BOOST_REQUIRE(i != m.end());
   BOOST_REQUIRE_THROW(m.add_target(rule_id, boost::function<void(const identifier&)>(lib_rule), rule_arg_names), std::exception);

   const rule_declaration& rd = i->second;
   BOOST_CHECK_EQUAL(rd.name(), rule_id);
   BOOST_CHECK(rd.result().type() == rule_argument_type::VOID);
   
   BOOST_REQUIRE_EQUAL(rd.arguments().size(), 1);
   BOOST_CHECK_EQUAL(rd.arguments()[0].type(), rule_argument_type::IDENTIFIER);
   BOOST_CHECK_EQUAL(rd.arguments()[0].name(), rule_arg_names[0]);
   BOOST_CHECK_EQUAL(rd.arguments()[0].is_optional(), false);
}

static void exe_rule(const identifier& id, 
                     const sources_decl& sources, 
                     const feature_set* requirements)
{

}

BOOST_AUTO_TEST_CASE(exe_rule_test)
{
   typedef boost::function<void(const identifier&, const sources_decl&, const feature_set*)> func_type;
   
   const identifier rule_id("exe");
   vector<identifier> rule_arg_names;
   rule_arg_names += "id", "sources", "requirements";

   rule_manager m;
   m.add_target(rule_id, func_type(exe_rule), rule_arg_names);

   rule_manager::const_iterator i = m.find(rule_id);
   BOOST_REQUIRE(i != m.end());
   BOOST_REQUIRE_THROW(m.add_target(rule_id, func_type(exe_rule), rule_arg_names), std::exception);

   const rule_declaration& rd = i->second;
   BOOST_CHECK_EQUAL(rd.name(), rule_id);
   BOOST_CHECK(rd.result().type() == rule_argument_type::VOID);

   BOOST_REQUIRE_EQUAL(rd.arguments().size(), 3);
   
   BOOST_CHECK_EQUAL(rd.arguments()[0].type(), rule_argument_type::IDENTIFIER);
   BOOST_CHECK_EQUAL(rd.arguments()[0].name(), rule_arg_names[0]);
   BOOST_CHECK_EQUAL(rd.arguments()[0].is_optional(), false);

   BOOST_CHECK_EQUAL(rd.arguments()[1].type(), rule_argument_type::SOURCES_DECL);
   BOOST_CHECK_EQUAL(rd.arguments()[1].name(), rule_arg_names[1]);
   BOOST_CHECK_EQUAL(rd.arguments()[1].is_optional(), false);

   BOOST_CHECK_EQUAL(rd.arguments()[2].type(), rule_argument_type::FEATURE_SET);
   BOOST_CHECK_EQUAL(rd.arguments()[2].name(), rule_arg_names[2]);
   BOOST_CHECK_EQUAL(rd.arguments()[2].is_optional(), true);
}
