#include "stdafx.h"
#include "enviroment.h"
#include "jcf_parser.h"
#include <hammer/src/engine.h>
#include <hammer/src/feature_registry.h>
#include <hammer/src/feature_set.h>
#include <hammer/src/generator_registry.h>
#include <hammer/src/basic_target.h>
#include <hammer/src/project_generators/msvc/msvc_project.h>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

struct generator_tests
{
   generator_tests() : engine_(test_data_path), ms_prj_(engine_), p_(0)
   {

   }

   void load()
   {
      
      BOOST_REQUIRE_NO_THROW(p_ = &engine_.load_project(fs::path("generator_tests") / test_name_));
      BOOST_REQUIRE(p_);
   } 
 
   void check()
   {
      BOOST_CHECK_NO_THROW(ms_prj_.generate());
//      BOOST_CHECK(checker_.walk(gtargets_, &engine_));
   }

   void instantiate(const char* target_name)
   {
      feature_set* build_request = engine_.feature_registry().make_set();
      build_request->insert("variant", "debug");
      build_request->insert("toolset", "msvc");
      p_->instantiate(target_name, *build_request, &itargets_);
      BOOST_REQUIRE(checker_.parse(test_data_path / "generator_tests" / test_name_ / "check.jcf"));
      BOOST_CHECK(checker_.walk(itargets_, &engine_));
   }
      
   void run_generators()
   {
      for(vector<basic_target*>::iterator i = itargets_.begin(), last = itargets_.end(); i != last; ++i)
      {
         boost::intrusive_ptr<build_node> r((**i).generate());
         ms_prj_.add_variant(r);
         nodes_.push_back(r);
      }
   }
  
   engine engine_;
   jcf_parser checker_;
   hammer::project_generators::msvc_project ms_prj_;
   const project* p_;
   vector<basic_target*> itargets_;
   vector<boost::intrusive_ptr<build_node> > nodes_;
   std::string test_name_;
};

BOOST_FIXTURE_TEST_CASE(simple_exe, generator_tests)
{
   test_name_ = "simple_exe";
   load();
   BOOST_REQUIRE_NO_THROW(instantiate("test"));
   BOOST_REQUIRE_NO_THROW(run_generators());
   check();
}

BOOST_FIXTURE_TEST_CASE(exe_and_static_lib, generator_tests)
{
   test_name_ = "exe_and_static_lib";
   load();
   BOOST_REQUIRE_NO_THROW(instantiate("test"));
   BOOST_REQUIRE_NO_THROW(run_generators());
   check();
}