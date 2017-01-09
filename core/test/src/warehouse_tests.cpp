#include <boost/test/unit_test.hpp>
#include <hammer/core/warehouse_impl.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/warehouse_target.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include "enviroment.h"
#include "options.h"
#include "jcf_parser.h"

using namespace std;
using namespace hammer;
using boost::unit_test::test_suite;
namespace fs = boost::filesystem;

namespace {

struct warehouse_test : complete_build_tests_environment
{
   warehouse_test(const bool remove_packages = true,
                  const bool remove_installed_packages = true);

   void add_all_libs();
   void setup_engine_warehouse();

   const fs::path packages_path_;
   const fs::path installed_packages_path_;
   const fs::path tests_path_;
   const fs::path libs_path_;
   unique_ptr<warehouse> warehouse_;
};

warehouse_test::warehouse_test(const bool remove_packages,
                               const bool remove_installed_packages)
   : packages_path_(test_data_path / "warehouse_tests/packages"),
     installed_packages_path_(test_data_path / "warehouse_tests/installed_packages"),
     tests_path_(test_data_path / "warehouse_tests/tests"),
     libs_path_(test_data_path / "warehouse_tests/libs")
{
   if (remove_packages) {
      fs::remove_all(packages_path_);
      fs::create_directory(packages_path_);
      {
         fs::ofstream f(packages_path_ / "packages.json");
         f << "[]";
         BOOST_REQUIRE(f);
      }
   }

   if (remove_installed_packages)
      fs::remove_all(installed_packages_path_);

   warehouse_.reset(new warehouse_impl("test", "file://" + packages_path_.string(), installed_packages_path_));
}

void warehouse_test::add_all_libs()
{
   warehouse_->add_to_packages(engine_.load_project(libs_path_ / "lib1/1.0/build"));
   warehouse_->add_to_packages(engine_.load_project(libs_path_ / "lib1/2.0/build"));
   warehouse_->add_to_packages(engine_.load_project(libs_path_ / "lib2/1.0/build"));
   warehouse_->add_to_packages(engine_.load_project(libs_path_ / "lib2/2.0/build"));
}

void warehouse_test::setup_engine_warehouse()
{
   const string script = (boost::format("setup-warehouse test : \"file://%1%\" : %2% ;") % packages_path_.string() % installed_packages_path_).str();
   engine_.load_hammer_script(script, "setup-warehouse");
   install_warehouse_rules(engine_.call_resolver(), engine_);
}

}

BOOST_AUTO_TEST_SUITE(warehouse_manual)

//BOOST_FIXTURE_TEST_CASE(double_push, warehouse_test)
//{
//   project& p = engine_.load_project(libs_path_ / "lib1/1.0/build");
//   BOOST_REQUIRE_NO_THROW(warehouse_->add_to_packages(p));
//   BOOST_REQUIRE_THROW(warehouse_->add_to_packages(p), std::exception);
//}

BOOST_AUTO_TEST_SUITE_END()

static
void test_function_phase_1(const fs::path& test_data_path)
{
   warehouse_test env(false, false);
   env.setup_engine_warehouse();

   options opts(test_data_path / "hamfile");
   if (opts.exists("skip"))
      return;

   project& p = env.engine_.load_project(test_data_path);
   if (!exists(test_data_path / "instantiation.jcf"))
      return;

   feature_set* build_request = env.engine_.feature_registry().make_set();
   build_request->join("toolset", "gcc");
   build_request->join("variant", "debug");

   vector<basic_target*> instantiated_targets;
   p.instantiate("test", *build_request, &instantiated_targets);

   try {
      build_nodes_t generated_nodes;
      for (const basic_target* bt : instantiated_targets) {
         build_nodes_t tmp = bt->generate();
         generated_nodes.insert(generated_nodes.end(), tmp.begin(), tmp.end());
      }
      BOOST_ERROR("Target generation should throw");
   } catch(const warehouse_unresolved_target_exception& e) {
   }

   const vector<warehouse::package_info> packages =
      env.engine_.warehouse().get_unresoved_targets_info(env.engine_, find_all_warehouse_unresolved_targets(instantiated_targets));

   null_warehouse_download_and_install notifier;
   BOOST_REQUIRE_NO_THROW(env.engine_.warehouse().download_and_install(env.engine_, packages, notifier));
}

static
void test_function_phase_2(const fs::path& test_data_path)
{
   // Phase 2
   warehouse_test env_2(false, false);
   env_2.setup_engine_warehouse();

   project& p = env_2.engine_.load_project(test_data_path);

   feature_set* build_request = env_2.engine_.feature_registry().make_set();
   build_request->join("toolset", "gcc");
   build_request->join("variant", "debug");

   vector<basic_target*> instantiated_targets;
   p.instantiate("test", *build_request, &instantiated_targets);

   // should be no throw here
   build_nodes_t generated_nodes;
   for (const basic_target* bt : instantiated_targets) {
      build_nodes_t tmp = bt->generate();
      generated_nodes.insert(generated_nodes.end(), tmp.begin(), tmp.end());
   }

   jcf_parser checker;
   BOOST_CHECK(checker.parse(test_data_path / "instantiation.jcf"));
   checker.walk(instantiated_targets, &env_2.engine_);
}

static
void test_function(const fs::path& test_data_path)
{
   {
      warehouse_test test;
      test.add_all_libs();
      test.warehouse_->update();
   }

   test_function_phase_1(test_data_path);
   test_function_phase_2(test_data_path);
}

static
void test_step(const fs::path& test_data_path,
               const unsigned step)
{
   // full test env rebuild only at beggining
   if (step == 1) {
      warehouse_test test;
      test.add_all_libs();
      test.warehouse_->update();
   }

   test_function_phase_1(test_data_path);
   test_function_phase_2(test_data_path);
}

static
test_suite*
add_steps(const fs::path& test_case_path)
{
   using namespace boost::unit_test;

   test_suite* ts = BOOST_TEST_SUITE(test_case_path.filename().string());
   unsigned step = 1;
   fs::path step_path = test_case_path / "steps" / to_string(step);
   while(is_directory(step_path)) {
      ts->add(make_test_case(boost::bind(&test_step, step_path, step), to_string(step)));

      ++step;
      step_path = test_case_path / "steps" / to_string(step);
   }

   return ts;
}

void init_warehouse_auto_tests(const fs::path& test_data_path)
{
   using namespace boost::unit_test;

   test_suite* ts = BOOST_TEST_SUITE("warehouse");
   for(fs::directory_iterator i(test_data_path / "warehouse_tests/tests"); i != fs::directory_iterator(); ++i) {
      if (is_directory(i->path() / "steps"))
         ts->add(add_steps(i->path()));
      else
         ts->add(make_test_case(boost::bind(&test_function, i->path()), i->path().filename().string()));
   }

   framework::master_test_suite().add(ts);
}
