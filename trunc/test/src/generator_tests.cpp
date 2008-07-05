#include "stdafx.h"
#include "enviroment.h"
#include "jcf_parser.h"
#include <hammer/src/engine.h>
#include <hammer/src/feature_registry.h>
#include <hammer/src/feature_set.h>
#include <hammer/src/generator_registry.h>
#include <hammer/src/basic_target.h>
#include <hammer/src/project_generators/msvc/msvc_solution.h>
#include <hammer/src/fs_helpers.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <set>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

static void compare_files(const fs::path& lhs, const fs::path& rhs, const fs::path& test_root)
{
   fs::ifstream lhs_f(lhs, std::ios::binary);
   fs::ifstream rhs_f(rhs, std::ios::binary);
   if (file_size(lhs) != file_size(rhs))
   {
      BOOST_CHECK_EQUAL(file_size(lhs), file_size(rhs));
      return;
   }
   
   bool result = std::equal(istreambuf_iterator<char>(lhs_f), istreambuf_iterator<char>(),
                            istreambuf_iterator<char>(rhs_f));
   
   if (!result)
   {
      ostringstream s;
      s << "File '" << relative_path(lhs, test_root) 
       << "' is not equal to '" << relative_path(rhs, test_root) << "'.";
      BOOST_CHECK_MESSAGE(result, s.str());
   }
}

namespace
{
   class test_msvc_solution : public hammer::project_generators::msvc_solution
   {
      public:
         test_msvc_solution(engine& e, const location_t& output_location) : msvc_solution(e, output_location)
         {
            fill(id_, id_ + 16, 0);
         }

      protected:
         virtual boost::guid generate_id() const
         {
            ++id_[15];
            return boost::guid(&id_[0], &id_[16]);
         }
      
      private:
         mutable unsigned char id_[16];
   };
}

struct generator_tests
{
   generator_tests() : p_(0)
   {

   }

   void load()
   {
      BOOST_REQUIRE_NO_THROW(p_ = &engine_.load_project(test_data_path / fs::path("generator_tests") / test_name_));
      BOOST_REQUIRE(p_);
   } 
 
   void check_msvc_solution()
   {
      fs::path etalon_files_path(test_data_path / "generator_tests" / test_name_ / "msvc80_solution_etalon");
      fs::path generated_files_path(test_data_path / "generator_tests" / test_name_);
      fs::recursive_directory_iterator i_etalon_files(etalon_files_path);
      fs::recursive_directory_iterator i_generated_files(generated_files_path);
      set<fs::path> etalon_files, generated_files;
      
      // gather files
      for(fs::recursive_directory_iterator last_etalon_files; i_etalon_files != last_etalon_files; ++i_etalon_files)
      {
         if (is_directory(*i_generated_files) &&
            i_etalon_files->leaf() == ".svn")
         {
            i_etalon_files.no_push();
            continue;
         }

         if (!is_directory(*i_etalon_files))
            etalon_files.insert(relative_path(*i_etalon_files, etalon_files_path));
      }

      for(fs::recursive_directory_iterator last_generated_files; i_generated_files != last_generated_files; ++i_generated_files)
      {
         if (is_directory(*i_generated_files) &&
             i_generated_files->leaf() == "msvc80_solution_etalon")
         {
            i_generated_files.no_push();
            continue;
         }

         if (!is_directory(*i_generated_files))
         {
            string ext = extension(*i_generated_files);
            if (ext == ".sln" || ext == ".vcproj")
               generated_files.insert(relative_path(*i_generated_files, generated_files_path));
         }
      }

      set<fs::path> unexpected_generated;
      set_difference(generated_files.begin(), generated_files.end(),
                     etalon_files.begin(), etalon_files.end(), 
                     insert_iterator<set<fs::path> >(unexpected_generated, unexpected_generated.end()));

      for(set<fs::path>::const_iterator i = unexpected_generated.begin(), last = unexpected_generated.end(); i != last; ++i)
      {
         ostringstream s;
         s << "Unexpected generated file '" << *i << "'.";
         BOOST_ERROR(s.str());
      }

      // remove unexpected from generated
      set<fs::path> tmp;
      set_difference(generated_files.begin(), generated_files.end(),\
                     unexpected_generated.begin(), unexpected_generated.end(),
                     insert_iterator<set<fs::path> >(tmp, tmp.end()));
      tmp.swap(generated_files);

      // checking 
      set<fs::path>::const_iterator 
         c_etalon_files = etalon_files.begin(), c_last_etalon_files = etalon_files.end(),
         c_generated_files = generated_files.begin(), c_last_generated_files = generated_files.end();
                        
      for(; c_etalon_files != c_last_etalon_files; ++c_etalon_files)
      {
         if (*c_etalon_files == *c_generated_files)
         {
            compare_files(etalon_files_path / *c_etalon_files, 
                          generated_files_path / *c_generated_files,
                          generated_files_path);
            ++c_generated_files;
         }
         else
         {
            ostringstream s;
            s << "File '" << *c_etalon_files << "' not founded.";
            BOOST_ERROR(s.str());
         }
      }
   }

   void check()
   {
      BOOST_CHECK_NO_THROW(msvc_solution_->write());
      check_msvc_solution();
//      BOOST_CHECK(checker_.walk(gtargets_, &engine_));
   }

   void instantiate(const char* target_name)
   {
      feature_set* build_request = engine_.feature_registry().make_set();
      build_request->join("variant", "release");
      build_request->join("toolset", "msvc");
      p_->instantiate(target_name, *build_request, &itargets_);
      BOOST_REQUIRE(checker_.parse(test_data_path / "generator_tests" / test_name_ / "check.jcf"));
      BOOST_CHECK(checker_.walk(itargets_, &engine_));
   }
      
   void run_generators()
   {
      msvc_solution_.reset(new test_msvc_solution(engine_, test_data_path / "generator_tests" / test_name_));
      for(vector<basic_target*>::iterator i = itargets_.begin(), last = itargets_.end(); i != last; ++i)
      {
         std::vector<boost::intrusive_ptr<build_node> > r((**i).generate());
         typedef std::vector<boost::intrusive_ptr<build_node> >::iterator iter;
         for(iter j = r.begin(), j_last = r.end(); j != j_last; ++j)
            msvc_solution_->add_target(*j);
         nodes_.insert(nodes_.end(), r.begin(), r.end());
      }
   }
  
   engine engine_;
   jcf_parser checker_;
   auto_ptr<test_msvc_solution> msvc_solution_;
   const project* p_;
   vector<basic_target*> itargets_;
   vector<boost::intrusive_ptr<build_node> > nodes_;
   std::string test_name_;
};

/*
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

BOOST_FIXTURE_TEST_CASE(path_features, generator_tests)
{
   test_name_ = "path_features";
   load();
   BOOST_REQUIRE_NO_THROW(instantiate("test"));
   BOOST_REQUIRE_NO_THROW(run_generators());
   check();
}

BOOST_FIXTURE_TEST_CASE(composite_features, generator_tests)
{
   test_name_ = "composite_features";
   load();
   BOOST_REQUIRE_NO_THROW(instantiate("test"));
   BOOST_REQUIRE_NO_THROW(run_generators());
   check();
}

BOOST_FIXTURE_TEST_CASE(cpp_libs, generator_tests)
{
   test_name_ = "cpp_libs";
   load();
   BOOST_REQUIRE_NO_THROW(instantiate("test"));
   BOOST_REQUIRE_NO_THROW(run_generators());
   check();
}

BOOST_FIXTURE_TEST_CASE(prebuilt_libs, generator_tests)
{
   test_name_ = "prebuilt_libs";
   load();
   BOOST_REQUIRE_NO_THROW(instantiate("test"));
   BOOST_REQUIRE_NO_THROW(run_generators());
   check();
}
*/

BOOST_FIXTURE_TEST_CASE(obj_meta_target, generator_tests)
{
   test_name_ = "obj_meta_target";
   load();
   BOOST_REQUIRE_NO_THROW(instantiate("test"));
   BOOST_REQUIRE_NO_THROW(run_generators());
   check();
}
