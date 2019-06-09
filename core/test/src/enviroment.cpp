#include "stdafx.h"
#include "enviroment.h"
#include <fstream>
#include <stdlib.h>
#include <boost/filesystem/operations.hpp>
#include <boost/bind.hpp>
#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/obj_generator.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/testing_generators.h>
#include <hammer/core/header_lib_generator.h>
#include <hammer/core/toolset_manager.h>
#include <hammer/core/types.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/builder.h>
#include <hammer/core/build_environment_impl.h>
#include <hammer/core/actuality_checker.h>
#include <hammer/core/c_scanner.h>
#include <hammer/core/scaner_manager.h>

#include <hammer/core/toolsets/gcc_toolset.h>
#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/toolsets/qt_toolset.h>
#include <hammer/core/htmpl/htmpl.h>
#include "options.h"

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;
using namespace boost::unit_test;

setuped_engine::setuped_engine(bool install_toolsets)
{
   hammer::types::register_standart_types(engine_.get_type_registry(), engine_.feature_registry());
   if (install_toolsets)
   {
      engine_.toolset_manager().add_toolset(engine_, std::unique_ptr<hammer::toolset>(new hammer::msvc_toolset()));
      hammer::location_t fake_location("fake_msvc_location");
      engine_.toolset_manager().init_toolset(engine_, "msvc", "11.0", &fake_location);
   }

   hammer::add_testing_generators(engine_, engine_.generators());
   hammer::add_header_lib_generator(engine_, engine_.generators());
}

complete_build_tests_environment::complete_build_tests_environment()
{
   hammer::types::register_standart_types(engine_.get_type_registry(), engine_.feature_registry());
   install_toolsets();

   auto c_scaner = std::make_shared<hammer::c_scanner>();
   engine_.scanner_manager().register_scanner(engine_.get_type_registry().get(types::CPP), c_scaner);
   engine_.scanner_manager().register_scanner(engine_.get_type_registry().get(types::C), c_scaner);
   engine_.scanner_manager().register_scanner(engine_.get_type_registry().get(types::RC), c_scaner);

   hammer::add_testing_generators(engine_, engine_.generators());
   hammer::add_header_lib_generator(engine_, engine_.generators());

   // Simplest way is to specify /home/username/user-config.ham as toolset configuring script
   const char* toolset_setup_script = getenv("HAMMER_TOOLSET_SETUP_SCRIPT");
   BOOST_REQUIRE_MESSAGE(toolset_setup_script, "You MUST set HAMMER_TOOLSET_SETUP_SCRIPT environment variable to script with toolset configuring instructions");

   BOOST_REQUIRE_NO_THROW(engine_.load_hammer_script(toolset_setup_script));
}

void complete_build_tests_environment::install_toolsets()
{
   engine_.toolset_manager().add_toolset(engine_, unique_ptr<toolset>(new gcc_toolset));
   engine_.toolset_manager().add_toolset(engine_, unique_ptr<toolset>(new msvc_toolset));
   engine_.toolset_manager().add_toolset(engine_, unique_ptr<toolset>(new qt_toolset));
   install_htmpl(engine_);
   engine_.generators().insert(std::auto_ptr<generator>(new obj_generator(engine_)));
}

void complete_build_tests_environment::run_test(const boost::filesystem::path& working_dir)
{
   project& p = engine_.load_project(working_dir);

   feature_set* build_request = engine_.feature_registry().make_set();
#if defined(_WIN32)
   const string default_toolset_name = "msvc";
#else
   const string default_toolset_name = "gcc";
#endif
   const feature_def& toolset_definition = engine_.feature_registry().get_def("toolset");
   const subfeature_def& toolset_version_def = toolset_definition.get_subfeature("version");
   // peek first configured as default
   const string& default_toolset_version = *toolset_version_def.legal_values(default_toolset_name).begin();
   build_request->join("toolset", (default_toolset_name + "-" + default_toolset_version).c_str());

   build_request->join("variant", "debug");

   vector<basic_target*> instantiated_targets;
   try {
      p.instantiate("test", *build_request, &instantiated_targets);
   } catch (const std::exception& e) {
      BOOST_FAIL(e.what());
   }

   BOOST_REQUIRE(!instantiated_targets.empty());

   build_nodes_t nodes_to_build;
   for (const basic_target* bt : instantiated_targets) {
      const build_nodes_t nodes = bt->generate();
      nodes_to_build.insert(nodes_to_build.end(), nodes.begin(), nodes.end());
   }

   build_environment_impl be(working_dir);

   // we run checker but not actualy use the result
   actuality_checker checker(engine_, be);
   checker.check(nodes_to_build);

   volatile bool interrupt_flag = false;
   builder builder(be, interrupt_flag, 1, true);

   options opts(working_dir / "hamfile");

   if (opts.exists("write-build-graph")) {
      ofstream graphviz((working_dir / "build.dot").string());
      builder.generate_graphviz(graphviz, nodes_to_build);
      graphviz.close();
   }

   builder::result build_result = builder.build(nodes_to_build);
   if (opts.exists("should-fail"))
      BOOST_REQUIRE_NE(build_result.failed_to_build_targets_, 0);
   else
      BOOST_REQUIRE_EQUAL(build_result.failed_to_build_targets_, 0);
}

void add_tests_from_filesystem(const boost::filesystem::path& test_data_path,
                               const std::string& test_suite_name,
                               std::function<void (const boost::filesystem::path& working_dir)> test_function)
{
   test_suite* ts = BOOST_TEST_SUITE(test_suite_name);
   for(fs::directory_iterator i(test_data_path); i != fs::directory_iterator(); ++i) {
      const fs::path test_path = i->path();
      if (!is_directory(test_path))
         continue;

      const string test_name = i->path().filename().string();
      ts->add(make_test_case([=] { test_function(test_path); }, test_name));
   }

   framework::master_test_suite().add(ts);
}
