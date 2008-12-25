#include "stdafx.h"
#include "enviroment.h"
#include <hammer/src/engine.h>
#include <hammer/src/feature_registry.h>
#include <hammer/src/basic_target.h>
#include <hammer/src/actuality_checker.h>
#include <hammer/src/build_environment.h>
#include <hammer/src/builder.h>
#include <hammer/src/build_action.h>
#include <hammer/src/feature_set.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

class toolset_test : public setuped_engine
{
   public:
      void do_test(const string& name);
};

namespace{
   class test_build_environment : public build_environment
   {
      public:
         test_build_environment(ostream& output, const location_t& cur_dir) 
            : output_(output),
              current_directory_(cur_dir)
         {
            current_directory_.normalize();
         }

         virtual bool run_shell_commands(const std::vector<std::string>& cmds) const
         {
            for(vector<string>::const_iterator i = cmds.begin(), last = cmds.end(); i != last; ++i)
               output_ << "[run shell command]: '" << *i << "'\n";
            
            return true;
         }

         virtual const location_t& current_directory() const
         {
            return current_directory_;
         }

         virtual void create_directories(const location_t& dir_to_create) const
         {
            output_ << "[mkdir] '" << dir_to_create << "'\n";
         }

         virtual void remove(const location_t& p) const
         {
            output_ << "[remove] '" << p << "'\n";
         }

         virtual void copy(const location_t& source, const location_t& destination) const
         {
            output_ << "[copy] '" << source << "' '" << destination << "'\n";
         }
         
      private:
         ostream& output_;
         location_t current_directory_;
   };
}

static void compare_files(const fs::path& lhs, const fs::path& rhs)
{
}

void toolset_test::do_test(const string& name)
{
   fs::path project_path(test_data_path / "toolset_tests" / name);
   BOOST_REQUIRE(exists(project_path / "hamfile"));
   const project& p = engine_.load_project(project_path);
   
   vector<string> test_cases;
   boost::regex file_mask("(.*?)\\.etalon");
   boost::smatch match;
   for(fs::directory_iterator i(project_path), last; i != last; ++i)
   {
      string filename = i->path().leaf();
      if (regex_match(filename, match, file_mask))
         test_cases.push_back(match[1]);
   }

   for(vector<string>::const_iterator i = test_cases.begin(), last = test_cases.end(); i != last; ++i)
   {
      BOOST_CHECKPOINT("Testing " + *i);
      fs::path etalon_file_path(project_path / (*i + ".etalon"));
      fs::ifstream etalon_file(etalon_file_path);
      BOOST_REQUIRE(etalon_file);
      string build_request_string;
      string target_name;
      BOOST_REQUIRE(etalon_file >> target_name >>  build_request_string);
      fs::path output_file_path(project_path / (*i + ".output"));
      fs::ofstream output_file(output_file_path, std::ios_base::trunc);
      BOOST_REQUIRE(output_file);
      output_file << target_name << ' ' << build_request_string << '\n';

      std::vector<basic_target*> instantiated_targets;
      const feature_set* build_request = parse_simple_set(build_request_string, engine_.feature_registry());
      p.instantiate(target_name, *build_request, &instantiated_targets);

      typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;
      nodes_t generated_nodes;
      for(vector<basic_target*>::const_iterator i = instantiated_targets.begin(), last = instantiated_targets.end(); i != last; ++i)
      {
         std::vector<boost::intrusive_ptr<build_node> > n((**i).generate());
         generated_nodes.insert(generated_nodes.end(), n.begin(), n.end());
      }

      actuality_checker checker;
      checker.check(generated_nodes);

      test_build_environment environment(output_file, project_path);
      builder b(environment);
      BOOST_REQUIRE_NO_THROW(b.build(generated_nodes));
      etalon_file.close();
      output_file.close();
      compare_files(etalon_file_path, output_file_path);
   }

   BOOST_CHECKPOINT("");
}

namespace{

/*
BOOST_FIXTURE_TEST_CASE(simple_exe, toolset_test)
{
   BOOST_REQUIRE_NO_THROW(do_test("simple_exe"));
}

BOOST_FIXTURE_TEST_CASE(exe_and_shared_lib, toolset_test)
{
   BOOST_REQUIRE_NO_THROW(do_test("exe_and_shared_lib"));
}

BOOST_FIXTURE_TEST_CASE(searched_lib, toolset_test)
{
   BOOST_REQUIRE_NO_THROW(do_test("searched_lib"));
}

BOOST_FIXTURE_TEST_CASE(exe_and_static_lib, toolset_test)
{
   BOOST_REQUIRE_NO_THROW(do_test("exe_and_static_lib"));
}

BOOST_FIXTURE_TEST_CASE(lib_dependencies, toolset_test)
{
   BOOST_REQUIRE_NO_THROW(do_test("lib_dependencies"));
}
*/

BOOST_FIXTURE_TEST_CASE(copy, toolset_test)
{
   BOOST_REQUIRE_NO_THROW(do_test("copy"));
}

}
