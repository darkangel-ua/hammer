#include "stdafx.h"
#include "enviroment.h"
#include "jcf_parser.h"
#include <hammer/src/engine.h>
#include <hammer/src/feature_registry.h>
#include <hammer/src/feature_set.h>
#include <hammer/src/generator_registry.h>
#include <hammer/src/basic_target.h>
#include <hammer/src/project_generators/msvc/msvc_solution.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <set>

using namespace hammer;
using namespace std;
namespace fs = boost::filesystem;

fs::path relative_path(fs::path p, const fs::path& relative_to) 
{ 
   using namespace fs;
   path current = relative_to; 

   // Trivial case 
   if(equivalent(current, p)) return p; 


   // Doesn't share a root 
   if(!equivalent(current.root_path(), p.root_path())) 
          return p; 


   // We don't care about the root anymore 
   // (and makes the rest easier) 
   current = current.relative_path(); 
   p = p.relative_path(); 


   path final(".", native); 


   path::iterator pit = p.begin(), 
                 cit = current.begin(); 
   // Find the shared directory 
   for(;pit != p.end() && cit != current.end(); ++pit, ++cit) 
          if(*pit != *cit) // May not be right 
                  break; 


   // Put needed parent dirs in 
   while(cit != current.end()) 
   { 
          final = ".." / final; 
          ++cit; 
   } 


   // Add the path from shared 
   while(pit != p.end()) 
          // Gah! Why doesn't *path::iterator return paths? 
          final /= path(*pit++, native); 


   // .normalize()? 
   return final; 
}

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
         test_msvc_solution(engine& e) : msvc_solution(e)
         {
         }

      protected:
         virtual boost::guid generate_id() const
         {
            ++id_[15];
            return boost::guid(&id_[0], &id_[16]);
         }
      
      private:
         static unsigned char id_[16];
   };

   unsigned char test_msvc_solution::id_[16] = {0};
}

struct generator_tests
{
   generator_tests() : engine_(test_data_path), msvc_solution_(engine_), p_(0)
   {

   }

   void load()
   {
      
      BOOST_REQUIRE_NO_THROW(p_ = &engine_.load_project(fs::path("generator_tests") / test_name_));
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
      BOOST_CHECK_NO_THROW(msvc_solution_.write());
      check_msvc_solution();
//      BOOST_CHECK(checker_.walk(gtargets_, &engine_));
   }

   void instantiate(const char* target_name)
   {
      feature_set* build_request = engine_.feature_registry().make_set();
      build_request->insert("variant", "release");
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
         msvc_solution_.add_target(r);
         nodes_.push_back(r);
      }
   }
  
   engine engine_;
   jcf_parser checker_;
   test_msvc_solution msvc_solution_;
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