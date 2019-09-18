#include <boost/test/unit_test.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <hammer/core/warehouse_impl.h>
#include <hammer/core/warehouse_manager.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/warehouse_target.h>
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

   const fs::path packages_path_;
   const fs::path installed_packages_path_;
   const fs::path tests_path_;
   const fs::path libs_path_;
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

   unique_ptr<warehouse> wh{ new warehouse_impl{ engine_, "test", "file://" + packages_path_.string(), installed_packages_path_ } };
   engine_.warehouse_manager().insert(std::move(wh));

   install_warehouse_rules(engine_);
}

void warehouse_test::add_all_libs()
{
   engine_.warehouse_manager().get_default()->add_to_packages(engine_.load_project(libs_path_ / "lib1/1.0"));
   engine_.warehouse_manager().get_default()->add_to_packages(engine_.load_project(libs_path_ / "lib1/2.0"));
   engine_.warehouse_manager().get_default()->add_to_packages(engine_.load_project(libs_path_ / "lib2/1.0"));
   engine_.warehouse_manager().get_default()->add_to_packages(engine_.load_project(libs_path_ / "lib2/2.0"));
   engine_.warehouse_manager().get_default()->add_to_packages(engine_.load_project(libs_path_ / "libA/1.0"));
   engine_.warehouse_manager().get_default()->add_to_packages(engine_.load_project(libs_path_ / "libB/1.0"));
}

}

static
void simulate_build_cmd(const fs::path& test_data_path) {
   warehouse_test env(false, false);

   options opts(test_data_path / "hamfile");
   if (opts.exists("skip"))
      return;

   const project& p = env.engine_.load_project(test_data_path);
   if (!exists(test_data_path / "instantiation.jcf"))
      return;

   feature_set* build_request = env.engine_.feature_registry().make_set();
#if defined(_WIN32)
   const string default_toolset_name = "msvc";
#else
   const string default_toolset_name = "gcc";
#endif
   const feature_def& toolset_definition = env.engine_.feature_registry().get_def("toolset");
   const subfeature_def& toolset_version_def = toolset_definition.get_subfeature("version");
   // peek first configured as default
   const string& default_toolset_version = *toolset_version_def.legal_values(default_toolset_name).begin();
   build_request->join("toolset", (default_toolset_name + "-" + default_toolset_version).c_str());

   build_request->join("variant", "debug");

   { // phase 1
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
         env.engine_.warehouse_manager().get_default()->get_unresoved_targets_info(env.engine_, find_all_warehouse_unresolved_targets(instantiated_targets));

      null_warehouse_download_and_install notifier;
      BOOST_REQUIRE_NO_THROW(env.engine_.warehouse_manager().get_default()->download_and_install(env.engine_, packages, notifier));
   }

   { // phase 2
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
      checker.walk(instantiated_targets, &env.engine_);
   }
}

static
void test_step(const fs::path& test_data_path,
               const unsigned step) {
   // full test env rebuild only at beggining
   if (step == 1) {
      warehouse_test test;
      test.add_all_libs();
      test.engine_.warehouse_manager().get_default()->update();
   }

   simulate_build_cmd(test_data_path);
}

static
test_suite*
add_steps(const fs::path& test_case_path) {
   using namespace boost::unit_test;

   test_suite* ts = BOOST_TEST_SUITE(test_case_path.filename().string());
   unsigned step = 1;
   fs::path step_path = test_case_path / "steps" / to_string(step);
   while(is_directory(step_path)) {
      ts->add(make_test_case(boost::bind(&test_step, step_path, step), to_string(step), step_path.string(), 0));

      ++step;
      step_path = test_case_path / "steps" / to_string(step);
   }

   return ts;
}

void init_warehouse_auto_tests(const fs::path& test_data_path) {
   using namespace boost::unit_test;

   test_suite* ts = BOOST_TEST_SUITE("warehouse");
   for(fs::directory_iterator i(test_data_path / "warehouse_tests/tests"); i != fs::directory_iterator(); ++i) {
      if (is_directory(i->path() / "steps"))
         ts->add(add_steps(i->path()));
      else
         ts->add(make_test_case(boost::bind(&test_step, i->path(), 1), i->path().filename().string(), i->path().string(), 0));
   }

   framework::master_test_suite().add(ts);
}
