#include "stdafx.h"
#include "enviroment.h"
#include <fstream>
#include <stdlib.h>
#include <boost/filesystem/operations.hpp>
#include <boost/bind.hpp>
#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/copy_generator.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/testing_generators.h>
#include <hammer/core/toolset_manager.h>
#include <hammer/core/types.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/builder.h>
#include <hammer/core/build_environment_impl.h>

#include <hammer/core/toolsets/gcc_toolset.h>
#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/toolsets/qt_toolset.h>
#include <hammer/core/htmpl/htmpl.h>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;
using namespace boost::unit_test;

setuped_engine::setuped_engine(bool install_toolsets)
{
   hammer::types::register_standart_types(engine_.get_type_registry(), engine_.feature_registry());
   if (install_toolsets)
   {
      engine_.toolset_manager().add_toolset(std::unique_ptr<hammer::toolset>(new hammer::msvc_toolset()));
      hammer::location_t empty;
      engine_.toolset_manager().init_toolset(engine_, "msvc", "8.0", &empty);
   }

   hammer::add_testing_generators(engine_, engine_.generators());

   std::unique_ptr<hammer::generator> g(new hammer::copy_generator(engine_));
   engine_.generators().insert(std::move(g));
}

static
void use_toolset_rule(project*,
                      engine& e,
                      pstring& toolset_name,
                      pstring& toolset_version,
                      pstring* toolset_home_)
{
   location_t toolset_home;
   if (toolset_home_ != NULL)
      toolset_home = toolset_home_->to_string();

   e.toolset_manager().init_toolset(e, toolset_name.to_string(), toolset_version.to_string(), toolset_home_ == NULL ? NULL : &toolset_home);
}

complete_build_tests_environment::complete_build_tests_environment()
{
   engine_.call_resolver().insert("use-toolset", boost::function<void (project*, pstring&, pstring&, pstring*)>(boost::bind(use_toolset_rule, _1, boost::ref(engine_), _2, _3, _4)));

   hammer::types::register_standart_types(engine_.get_type_registry(), engine_.feature_registry());
   install_toolsets();

   // Simplest way is to specify /home/username/user-config.ham as toolset configuring script
   const char* toolset_setup_script = getenv("HAMMER_TOOLSET_SETUP_SCRIPT");
   BOOST_REQUIRE_MESSAGE(toolset_setup_script, "You MUST set HAMMER_TOOLSET_SETUP_SCRIPT environment variable to script with toolset configuring instructions");

   BOOST_REQUIRE_NO_THROW(engine_.load_hammer_script(toolset_setup_script));
}

void complete_build_tests_environment::install_toolsets()
{
   engine_.toolset_manager().add_toolset(unique_ptr<toolset>(new gcc_toolset));
   engine_.toolset_manager().add_toolset(unique_ptr<toolset>(new msvc_toolset));
   engine_.toolset_manager().add_toolset(unique_ptr<toolset>(new qt_toolset));
   install_htmpl(engine_);
}

void complete_build_tests_environment::run_test(const boost::filesystem::path& working_dir)
{
   project& p = engine_.load_project(working_dir);

   feature_set* build_request = engine_.feature_registry().make_set();
#ifdef _WIN32
   build_request->join("toolset", "msvc");
#else
   build_request->join("toolset", "gcc");
#endif
   build_request->join("variant", "debug");

   vector<basic_target*> instantiated_targets;
   BOOST_REQUIRE_NO_THROW(p.instantiate("test", *build_request, &instantiated_targets));
   BOOST_REQUIRE(!instantiated_targets.empty());

   build_nodes_t nodes_to_build;
   for (const basic_target* bt : instantiated_targets) {
      const build_nodes_t nodes = bt->generate();
      nodes_to_build.insert(nodes_to_build.end(), nodes.begin(), nodes.end());
   }

   build_environment_impl be(fs::current_path());
   volatile bool interrupt_flag = false;
   builder builder(be, interrupt_flag, 1, true);

//   FIXME: will enable this later using options from hamfile
//   ofstream graphviz((working_dir / "build.dot").string());
//   builder.generate_graphviz(graphviz, nodes_to_build);
//   graphviz.close();

   builder::result build_result = builder.build(nodes_to_build);
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
