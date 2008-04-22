#include "stdafx.h"
#include "enviroment.h"
#include "jcf_parser.h"
#include <hammer/src/engine.h>
#include <hammer/src/feature_registry.h>
#include <hammer/src/feature_set.h>
#include <hammer/src/generator_registry.h>
#include <hammer/src/basic_target.h>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

struct generator_tests
{
   generator_tests() : engine_(test_data_path), p_(0)
   {

   }

   void load(const char* test_dir)
   {
      
      BOOST_REQUIRE_NO_THROW(p_ = &engine_.load_project(fs::path("generator_tests") / test_dir));
      BOOST_REQUIRE(p_);
   } 

   void check(const char* test_name)
   {
      BOOST_CHECK(checker_.parse(test_data_path / "generator_tests" / test_name / "check.jcf"));
//      BOOST_CHECK(checker_.walk(gtargets_, &engine_));
   }

   void instantiate(const char* target_name)
   {
      feature_set* build_request = engine_.feature_registry().make_set();
      build_request->insert("variant", "debug");
      build_request->insert("toolset", "msvc");
      itargets_ = p_->instantiate(target_name, *build_request);
   }
      
   void run_generators()
   {
      for(vector<basic_target*>::iterator i = itargets_.begin(), last = itargets_.end(); i != last; ++i)
      {
         boost::intrusive_ptr<build_node> r((**i).generate());

         nodes_.push_back(r);
      }
   }
  
   engine engine_;
   jcf_parser checker_;
   const project* p_;
   vector<basic_target*> itargets_;
   vector<boost::intrusive_ptr<build_node> > nodes_;
};

BOOST_FIXTURE_TEST_CASE(simple_exe, generator_tests)
{
   load("simple_exe");
   BOOST_REQUIRE_NO_THROW(instantiate("test"));
   BOOST_REQUIRE_NO_THROW(run_generators());
   check("simple_exe");
}