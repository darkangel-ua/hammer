#include "stdafx.h"
#include "enviroment.h"
#include <hammer/src/engine.h>
#include <hammer/src/feature_registry.h>
#include <hammer/src/basic_target.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

class toolset_test
{
   public:
      void do_test(const string& name);

      engine engine_;
};

void toolset_test::do_test(const string& name)
{
   fs::path project_path(test_data_path / "toolset_tests" / name);
   BOOST_REQUIRE(exists(project_path / "hamfile"));
   const project& p = engine_.load_project(project_path);
   std::vector<basic_target*> instantiated_targets;
   p.instantiate("test", *engine_.feature_registry().make_set(), &instantiated_targets);

   typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;
   nodes_t generated_nodes;
   for(vector<basic_target*>::const_iterator i = instantiated_targets.begin(), last = instantiated_targets.end(); i != last; ++i)
   {
      std::vector<boost::intrusive_ptr<build_node> > n((**i).generate());
      generated_nodes.insert(generated_nodes.end(), n.begin(), n.end());
   }

   for(nodes_t::const_iterator i = generated_nodes.begin(), last = generated_nodes.end(); i != last; ++i)
   {
      
   }
}

BOOST_FIXTURE_TEST_CASE(simple_exe, toolset_test)
{
   BOOST_REQUIRE_NO_THROW(do_test("simple_exe"));
}
