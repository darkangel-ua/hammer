#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/optional/optional.hpp>
#include <hammer/core/rule_manager.h>
#include <hammer/core/diagnostic.h>

using namespace std; 
using namespace hammer;
using namespace hammer::parscore;
using namespace boost::unit_test;

class test_feature_set;
class test_sources;

namespace hammer {
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(test_sources, sources);
   HAMMER_RULE_MANAGER_SIMPLE_TYPE(test_feature_set, feature_set);

   class project{};
}

static
void lib_rule(target_invocation_context& ctx,
              const identifier& id)
{

}

BOOST_AUTO_TEST_CASE(lib_rule_test)
{
   const identifier rule_id("lib");
   vector<rule_argument_decl> rule_arg_names = { "id" };

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
   BOOST_CHECK_EQUAL(rd.arguments()[1].name(), rule_arg_names[0].name_);
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
   vector<rule_argument_decl> rule_arg_names = { "id", "sources", "requirements" };

   rule_manager m;
   m.add_rule(rule_id, exe_rule, rule_arg_names);

   rule_manager::const_iterator i = m.find(rule_id);
   BOOST_REQUIRE(i != m.end());
   BOOST_REQUIRE_THROW(m.add_rule(rule_id, exe_rule, rule_arg_names), std::exception);

   const rule_declaration& rd = i->second;
   BOOST_CHECK_EQUAL(rd.name(), rule_id);
   BOOST_CHECK(rd.result().type() == rule_argument_type::void_);

   BOOST_REQUIRE_EQUAL(rd.arguments().size(), 4);
   
   BOOST_CHECK(rd.arguments()[1].type() == rule_argument_type::identifier);
   BOOST_CHECK_EQUAL(rd.arguments()[1].name(), rule_arg_names[0].name_);
   BOOST_CHECK_EQUAL(rd.arguments()[1].is_optional(), false);

   BOOST_CHECK(rd.arguments()[2].type() == rule_argument_type::sources);
   BOOST_CHECK_EQUAL(rd.arguments()[2].name(), rule_arg_names[1].name_);
   BOOST_CHECK_EQUAL(rd.arguments()[2].is_optional(), false);

   BOOST_CHECK(rd.arguments()[3].type() == rule_argument_type::feature_set);
   BOOST_CHECK_EQUAL(rd.arguments()[3].name(), rule_arg_names[2].name_);
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
   vector<rule_argument_decl> rule_arg_names = { "id_1", "id_2", "id_3" };

   rule_manager m;
   m.add_rule(rule_id, invoke_function_1, rule_arg_names);
   const rule_declaration& rd = m.find(rule_id)->second;

   hammer::project p;
   ostringstream s;
   streamed_diagnostic diag("invoke_test_1", true, s);
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
invoke_function_2(target_invocation_context& ctx,
                  const identifier& id_1)
{
   invoke_function_2_invoked = true;
   BOOST_CHECK_EQUAL(ctx.local_, true);
   BOOST_CHECK_EQUAL(ctx.explicit_, false);
   return std::unique_ptr<identifier>(new identifier("123qwe"));
}

BOOST_AUTO_TEST_CASE(invoke_test_2)
{
   const identifier rule_id("invoke_function_2");
   vector<rule_argument_decl> rule_arg_names = { "id_1" };

   rule_manager m;
   m.add_target(rule_id, invoke_function_2, rule_arg_names);
   const rule_declaration& rd = m.find(rule_id)->second;

   hammer::project p;
   ostringstream s;
   streamed_diagnostic diag("invoke_test_1", true, s);
   target_invocation_context ctx = {p, diag, m, true, false};
   identifier id_1("foo");

   rule_manager_arg_ptr arg_0(new rule_manager_arg<target_invocation_context>(ctx));
   rule_manager_arg_ptr arg_1(new rule_manager_arg<identifier>(id_1));

   rule_manager_arguments_t args;
   args.push_back(move(arg_0));
   args.push_back(move(arg_1));

   rule_manager_arg_ptr result;
   BOOST_REQUIRE_NO_THROW(result = rd.invoke(args));

   BOOST_CHECK_EQUAL(invoke_function_2_invoked, true);
   BOOST_CHECK_EQUAL(*static_cast<identifier*>(result->value()), identifier("123qwe"));
}

static bool id_list_rule_invoked = false;

static
void id_list_rule(invocation_context& ctx,
                  const one_or_list<identifier>& ids)
{
   id_list_rule_invoked = true;
   BOOST_REQUIRE_EQUAL(ids.value_.size(), 1);
   BOOST_CHECK_EQUAL(ids.value_.front(), "foo");
}

BOOST_AUTO_TEST_CASE(invoke_test_id_list)
{
   const identifier rule_id("id_list_rule");

   rule_manager m;
   m.add_rule(rule_id, id_list_rule, { "ids" });
   const rule_declaration& rd = m.find(rule_id)->second;

   hammer::project p;
   ostringstream s;
   streamed_diagnostic diag("invoke_test_1", true, s);
   target_invocation_context ctx = {p, diag, m, true, false};
   identifier id_1("foo");

   rule_manager_arg_ptr arg_0(new rule_manager_arg<invocation_context>(ctx));
   rule_manager_arg_ptr arg_1(new rule_manager_arg<one_or_list<identifier>>({{id_1}}));

   rule_manager_arguments_t args;
   args.push_back(move(arg_0));
   args.push_back(move(arg_1));

   rule_manager_arg_ptr result;
   BOOST_REQUIRE_NO_THROW(result = rd.invoke(args));

   BOOST_CHECK_EQUAL(id_list_rule_invoked, true);
}

struct simple_struct {
   identifier id_1_;
   boost::optional<identifier> id_2_;
};

namespace hammer {

template<>
struct rule_argument_type_info<simple_struct> {
   static
   std::unique_ptr<simple_struct>
   constructor(const identifier& id_1,
               const identifier* id_2) {
      return std::unique_ptr<simple_struct>(new simple_struct{id_1, id_2 ? boost::optional<identifier>{*id_2} : boost::optional<identifier>{}});
   }

   static
   rule_argument_type_desc
   ast_type() {
      return make_rule_argument_struct_desc("simple_struct", constructor, {"id_1", "id_2"});
   }
};

}

static
bool struct_rule_invoked = false;

static
void struct_rule(invocation_context& ctx,
                 const simple_struct& s)
{
   struct_rule_invoked = true;
   BOOST_REQUIRE_EQUAL(s.id_1_, "id_1");
   BOOST_REQUIRE(s.id_2_);
   BOOST_REQUIRE_EQUAL(*s.id_2_, "id_2");
}

BOOST_AUTO_TEST_CASE(invoke_test_struct)
{
   const identifier rule_id("struct_rule");

   rule_manager m;
   m.add_rule(rule_id, struct_rule, { "s" });
   const rule_declaration& rd = m.find(rule_id)->second;

   hammer::project p;
   ostringstream s;
   streamed_diagnostic diag("invoke_test_struct", true, s);
   target_invocation_context ctx = {p, diag, m, true, false};
   identifier id_1("id_1");
   identifier id_2{"id_2"};

   rule_manager_arg_ptr arg_0(new rule_manager_arg<invocation_context>(ctx));
   rule_manager_arg_ptr arg_1(new rule_manager_arg<simple_struct>({id_1, id_2}));

   rule_manager_arguments_t args;
   args.push_back(move(arg_0));
   args.push_back(move(arg_1));

   rule_manager_arg_ptr result;
   BOOST_REQUIRE_NO_THROW(result = rd.invoke(args));

   BOOST_CHECK_EQUAL(id_list_rule_invoked, true);
}
