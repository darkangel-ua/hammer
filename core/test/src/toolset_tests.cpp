#include "stdafx.h"
#include "enviroment.h"
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/actuality_checker.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/builder.h>
#include <hammer/core/build_action.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/main_target.h>

#include <hammer/core/toolset_manager.h>
#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/toolsets/gcc_toolset.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

class test_msvc_toolset : public hammer::msvc_toolset
{
   protected:
      virtual msvc_8_0_data resolve_8_0_data(const location_t* toolset_home) const
      {
         msvc_8_0_data result;
         result.setup_script_ = "vcvars32.bat";
         result.compiler_ = "cl.exe";
         result.librarian_ = "lib.exe";
         result.linker_ = "link.exe";
         result.manifest_tool_ = "mt.exe";

         return result;
      }
};

class toolset_test : public setuped_engine
{
   public:
      toolset_test() : setuped_engine(false)
      {
         engine_.toolset_manager().add_toolset(unique_ptr<toolset>(new test_msvc_toolset));
         engine_.toolset_manager().add_toolset(unique_ptr<toolset>(new gcc_toolset));
         engine_.toolset_manager().init_toolset(engine_, "msvc", "8.0");
         engine_.toolset_manager().init_toolset(engine_, "gcc", "");
      }

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

         virtual bool run_shell_commands(const std::vector<std::string>& cmds, const location_t& working_dir) const
         {
            for(vector<string>::const_iterator i = cmds.begin(), last = cmds.end(); i != last; ++i)
               output_ << "[run shell command]: '" << *i << "'\n";
            
            return true;
         }

         virtual bool run_shell_commands(std::string& captured_output, const std::vector<std::string>& cmds, const location_t& working_dir) const
         {
            return run_shell_commands(cmds, working_dir);
         }

         virtual bool run_shell_commands(std::ostream& captured_output_stream, const std::vector<std::string>& cmds, const location_t& working_dir) const
         {
            return run_shell_commands(cmds, working_dir);
         }

         bool run_shell_commands(std::ostream& captured_output_stream,
                                 std::ostream& captured_error_stream,
                                 const std::vector<std::string>& cmds,
                                 const location_t& working_dir) const override
         {
            return run_shell_commands(cmds, working_dir);
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

         virtual void remove_file_by_pattern(const location_t& dir, const std::string& pattern) const
         {
            output_ << "[remove_file_by_pattern] dir = '" << dir << "' pattern = '" << pattern << "'\n";
         }

         virtual void copy(const location_t& source, const location_t& destination) const
         {
            output_ << "[copy] '" << source << "' '" << destination << "'\n";
         }

         virtual bool write_tag_file(const std::string& filename, const std::string& content) const
         {
            output_ << "[write_tag_file] filename ='" << filename << "', content = '" << content << "'\n";
            return true;
         }

         std::unique_ptr<ostream> create_output_file(const char* filename, std::ios_base::openmode mode) const
         {
            return std::unique_ptr<ostream>(new ostringstream);
         }

         virtual location_t working_directory(const basic_target& t) const
         {
            return t.get_main_target()->location();
         }

         virtual const location_t* cache_directory() const
         {
            return NULL;
         }
         
         virtual std::ostream& output_stream() const
         {
            return null_output_;
         }

         std::ostream& error_stream() const override
         {
            return null_output_;
         }

      private:
         mutable std::stringstream null_output_;
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
      string filename = i->path().leaf().string();
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

      test_build_environment environment(output_file, project_path);
      actuality_checker checker(engine_, environment);
      checker.check(generated_nodes);

      volatile bool interrupt_flag = false;
      builder b(environment, interrupt_flag, 1, true);
      BOOST_REQUIRE_NO_THROW(b.build(generated_nodes));
      etalon_file.close();
      output_file.close();
      compare_files(etalon_file_path, output_file_path);
   }

   BOOST_CHECKPOINT("");
}
/*
namespace{

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

BOOST_FIXTURE_TEST_CASE(dependency, toolset_test)
{
   BOOST_REQUIRE_NO_THROW(do_test("dependency"));
}

BOOST_FIXTURE_TEST_CASE(testing_run, toolset_test)
{
   BOOST_REQUIRE_NO_THROW(do_test("testing_run"));
}

BOOST_FIXTURE_TEST_CASE(copy, toolset_test)
{
   BOOST_REQUIRE_NO_THROW(do_test("copy"));
}

BOOST_FIXTURE_TEST_CASE(file, toolset_test)
{
   BOOST_REQUIRE_NO_THROW(do_test("file"));
}

}
*/
