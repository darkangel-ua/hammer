#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/bind.hpp>
#include <yaml-cpp/yaml.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/target_type.h>
#include <hammer/core/basic_build_target.h>
#include "options.h"
#include "enviroment.h"

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;
using namespace boost::unit_test;

static
void verify(const build_node& node,
            const YAML::Node& vn,
            const string& path) {  
   build_node::sources_t missed_sources{ node.sources_ };
   const auto& vsources = vn["sources"];
   if (!vsources) {
      // single line in node description means its a node with only one product and zero sources
      if (node.products_.size() != 1) {
         BOOST_ERROR(path + " must be a node with only one product");
         return;
      }

      const string target_to_search = vn.begin()->first.as<string>();
      const string target_type_to_search = vn.begin()->second.as<string>();
      if (node.products_[0]->name() != target_to_search || node.products_[0]->type().tag().name() != target_type_to_search)
         BOOST_ERROR("Unexpected product found: " + target_to_search + "(" + target_type_to_search + ") at " + path);
   } else {
      for (const auto& vs : vsources) {
         const string target_to_search = vs.begin()->first.as<string>();
         const string target_type_to_search = vs.begin()->second.as<string>();
         auto i = find_if(node.sources_.begin(), node.sources_.end(), [&](const build_node::source_t& s) {
            return s.source_target_->name() == target_to_search && s.source_target_->type().tag().name() == target_type_to_search;
         });

         if (i == node.sources_.end())
            BOOST_ERROR(path + "/sources/" + target_to_search + "(" + target_type_to_search + ") - not found in build tree");
         else {
            missed_sources.erase(find_if(missed_sources.begin(), missed_sources.end(), [&](const build_node::source_t& s) {
               return s.source_target_ == i->source_target_ && s.source_node_ == i->source_node_;
            }));

            verify(*i->source_node_, vs, path + "/sources/" + target_to_search + "(" + target_type_to_search + ")");
         }
      }

      for (const auto& ms : missed_sources)
         BOOST_ERROR("Extra source found: " + path + "/sources/" + ms.source_target_->name() + "(" + ms.source_target_->type().tag().name() + ")");
   }
}

static
void verify(const build_nodes_t& nodes,
            const YAML::Node& verification_tree) {

   for (const auto& vn : verification_tree) {
      const string target_to_search = vn.begin()->first.as<string>();
      const string target_type_to_search = vn.begin()->second.as<string>();
      auto i = find_if(nodes.begin(), nodes.end(), [&](const build_node_ptr& node) {
         auto i = find_if(node->products_.begin(), node->products_.end(), [&](const basic_build_target* bt) {
            return bt->name() == target_to_search && bt->type().tag().name() == target_type_to_search;
         });

         return i != node->products_.end();
      });

      if (i == nodes.end())
         BOOST_ERROR("Failed to find top level node " + target_to_search + "(" + target_type_to_search + ")");

      verify(**i, vn, "nodes/" + target_to_search + "(" + target_type_to_search + ")");
   }
}

struct generation_tests : public complete_build_tests_environment {
   build_nodes_t
   generate(const vector<basic_target*>& targets) {
      build_nodes_t result;
      for (const basic_target* t : targets ) {
         build_nodes_t nodes = t->generate();
         result.insert(result.end(), nodes.begin(), nodes.end());
      }

      return result;
   }

   void run_test(const fs::path& test_data_path) {
      options opts{test_data_path / "hamfile"};
      if (opts.exists("skip"))
         return;

      BOOST_CHECK_NO_THROW(engine_.load_project(test_data_path));

      if (opts.exists("load-should-fail") && opts["load-should-fail"] == "true") {
         BOOST_CHECK_THROW(engine_.load_project(test_data_path), std::exception);
         return;
      }

      project& p = engine_.load_project(test_data_path);

      // if instantiation.jcf presents than run checker
      // FIXME: else we just parse project for now
      const fs::path nodes_yaml = test_data_path / "nodes.yaml";
      if (opts.exists("should-fail") || exists(nodes_yaml)) {
         fs::ifstream f(nodes_yaml);
         if (!f)
            BOOST_FAIL("Can't open '" + nodes_yaml.string() + "' file");

         YAML::Node verification_tree = YAML::Load(f);

         feature_set* build_request = engine_.feature_registry().make_set();
         for (const auto& brf : verification_tree["build-request"])
            build_request->join(brf.begin()->first.as<string>().c_str(), brf.begin()->second.as<string>().c_str());

         vector<basic_target*> instantiated_targets;
         p.instantiate("test", *build_request, &instantiated_targets);
         if (opts.exists("should-fail"))
            BOOST_CHECK_THROW(generate(instantiated_targets), std::exception)
         else {
            build_nodes_t nodes = generate(instantiated_targets);
            BOOST_REQUIRE_NO_THROW(verify(nodes, verification_tree["nodes"]));
         }
      }
   }

   string name_;
};

static
void test_function(const fs::path& test_data_path) {
   generation_tests tests;
   tests.run_test(test_data_path);
}

void init_generation_tests(const fs::path& test_data_path) {
   test_suite* ts = BOOST_TEST_SUITE("generation");
   for (fs::directory_iterator i(test_data_path / "generation_tests"); i != fs::directory_iterator(); ++i)
      ts->add(make_test_case(boost::bind(&test_function, i->path()), i->path().filename().string()));

   framework::master_test_suite().add(ts);
}

