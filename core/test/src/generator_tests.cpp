#include "stdafx.h"
#include <set>
#include <algorithm>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/guid.hpp>
#include <boost/bind.hpp>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/project_generators/msvc_solution.h>
#include <hammer/core/fs_helpers.h>
#include "enviroment.h"
#include "jcf_parser.h"
#include "options.h"

// FIXME: this is outdated and I even don't know how it works
// I leave it here just in case
using namespace hammer;
using namespace hammer::project_generators;
using namespace std;
using namespace boost::unit_test;
namespace fs = boost::filesystem;

static void compare_files(const fs::path& lhs, const fs::path& rhs, const fs::path& test_root)
{
   fs::ifstream lhs_f(lhs, std::ios::binary);
   fs::ifstream rhs_f(rhs, std::ios::binary);
   if (file_size(lhs) != file_size(rhs))
   {
      BOOST_CHECK_MESSAGE(file_size(lhs) == file_size(rhs), lhs.string() + " != " + rhs.string());
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

static bool less_target(const basic_target* lhs, const basic_target* rhs)
{
   location_t lhs_id = lhs->location() / lhs->name().to_string();
   location_t rhs_id = rhs->location() / rhs->name().to_string();
   lhs_id.normalize();
   rhs_id.normalize();

   return lhs_id < rhs_id;
}

namespace hammer{

static bool operator < (const build_node::source_t& lhs, 
                        const build_node::source_t& rhs)
{
   return less_target(lhs.source_target_, rhs.source_target_);
}

}

namespace
{
   class test_msvc_solution : public hammer::project_generators::msvc_solution
   {
      public:
         test_msvc_solution(const project& source_project, const location_t& output_location, 
                            generation_mode::value mode) : msvc_solution(source_project, output_location, mode)
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

static int compare_sources(const build_node::sources_t& lhs,
                            const build_node::sources_t& rhs)
{
   for(build_node::sources_t::const_iterator i = lhs.begin(), i_r = rhs.begin(), last = lhs.end(); i != last; ++i, ++i_r)
   {
      location_t lhs_id = i->source_target_->location() / i->source_target_->name().to_string();
      location_t rhs_id = i_r->source_target_->location() / i_r->source_target_->name().to_string();
      lhs_id.normalize();
      rhs_id.normalize();
      
      if (lhs_id != rhs_id)
         return lhs_id < rhs_id ? 1 : -1;
   }

   return 0;
}

static int compare_products(const build_node::targets_t& lhs,
                            const build_node::targets_t& rhs)
{
   for(build_node::targets_t::const_iterator i = lhs.begin(), i_r = rhs.begin(), last = lhs.end(); i != last; ++i, ++i_r)
   {
      location_t lhs_id = (**i).location() / (**i).name().to_string();
      location_t rhs_id = (**i_r).location() / (**i_r).name().to_string();
      lhs_id.normalize();
      rhs_id.normalize();

      if (lhs_id != rhs_id)
         return lhs_id < rhs_id ? 1 : -1;
   }   

   return 0;
}

static
bool less_node(const boost::intrusive_ptr<build_node>& lhs, 
               const boost::intrusive_ptr<build_node>& rhs);

static int compare_dependencies(const build_node::nodes_t& lhs,
                                const build_node::nodes_t& rhs)
{
   for(build_node::nodes_t::const_iterator i = lhs.begin(), i_r = rhs.begin(), last = lhs.end(); i != last; ++i, ++i_r)
   {
      if (less_node(*i, *i_r) != less_node(*i_r, *i))
         return less_node(*i, *i_r);
   }

   return 0;
}

static bool less_node(const boost::intrusive_ptr<build_node>& lhs, 
                      const boost::intrusive_ptr<build_node>& rhs)
{
   if (lhs->sources_.size() == rhs->sources_.size())
   {
      if (int res = compare_sources(lhs->sources_, rhs->sources_))
         return res == 1 ? true : false;
   }
   else
      return lhs->sources_.size() < rhs->sources_.size();
   
   if (lhs->products_.size() == rhs->products_.size())
   {
      if (int res = compare_products(lhs->products_, rhs->products_))
         return res == 1 ? true : false;
   }
   else
      return lhs->products_.size() < rhs->products_.size();

   if (lhs->dependencies_.size() == rhs->dependencies_.size())
   {
      if (int res = compare_dependencies(lhs->dependencies_, rhs->dependencies_))
         return res == 1 ? true : false;
   }
   
   return false;
}

struct generator_tests : setuped_engine
{
   generator_tests() : p_(0), generators_output_dir_name_(".hammer")
   {

   }

   void check_msvc_solution(const fs::path& test_data_path)
   {
      fs::path etalon_files_path(test_data_path  / "hammer_etalon");
      fs::path generated_files_path(test_data_path );
      fs::recursive_directory_iterator i_etalon_files(etalon_files_path);
      fs::recursive_directory_iterator i_generated_files(generated_files_path);
      set<fs::path> etalon_files, generated_files;
      
      // gather files
      for(fs::recursive_directory_iterator last_etalon_files; i_etalon_files != last_etalon_files; ++i_etalon_files)
      {
         if (!is_directory(*i_etalon_files))
            etalon_files.insert(relative_path(*i_etalon_files, etalon_files_path));
      }

      for(fs::recursive_directory_iterator last_generated_files; i_generated_files != last_generated_files; ++i_generated_files)
      {
         if (is_directory(*i_generated_files) &&
             i_generated_files->path() == "hammer_etalon")
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

   void check(const fs::path& test_data_path)
   {
      BOOST_CHECK_NO_THROW(msvc_solution_->write());
      check_msvc_solution(test_data_path);
   }

   void instantiate(const fs::path& test_data_path, const string& target_name)
   {
      feature_set* build_request = engine_.feature_registry().make_set();
      build_request->join("variant", "release");
      build_request->join("toolset", "msvc");
      p_->instantiate(target_name.c_str(), *build_request, &itargets_);

      if (exists(test_data_path / "check.jcf"))
      {
         BOOST_REQUIRE(checker_.parse(test_data_path / "check.jcf"));
         BOOST_CHECK(checker_.walk(itargets_, &engine_));
      }
   }
   
   void sort_node(build_node& node)
   {
      std::sort(node.sources_.begin(), node.sources_.end());
      std::sort(node.products_.begin(), node.products_.end(), &less_target);
      sort_tree(node.dependencies_);
   }

   void sort_tree(build_node::nodes_t& nodes)
   {
      for(build_node::nodes_t::iterator i = nodes.begin(), last = nodes.end(); i != last; ++i)
         sort_node(**i);

      std::sort(nodes.begin(), nodes.end(), &less_node);
   }

   void run_generators(const fs::path& test_data_path, msvc_solution::generation_mode::value mode = msvc_solution::generation_mode::NON_LOCAL)
   {
      msvc_solution_.reset(new test_msvc_solution(*p_, test_data_path / generators_output_dir_name_, mode));
      for(vector<basic_target*>::iterator i = itargets_.begin(), last = itargets_.end(); i != last; ++i)
      {
         std::vector<boost::intrusive_ptr<build_node> > r((**i).generate());
         sort_tree(r);

         typedef std::vector<boost::intrusive_ptr<build_node> >::iterator iter;
         for(iter j = r.begin(), j_last = r.end(); j != j_last; ++j)
            msvc_solution_->add_target(*j);
         nodes_.insert(nodes_.end(), r.begin(), r.end());
      }
   }

   void run_test(const fs::path& test_data_path)
   {
      msvc_solution::generation_mode::value mode = msvc_solution::generation_mode::NON_LOCAL;
      string testing_target_name = "test";
      options opts(test_data_path / "hamfile");
      if (opts.exists("skip"))
         return;
     
      if (opts.exists("output_dir"))
         generators_output_dir_name_ = opts["output_dir"];

      if (opts.exists("local"))
         mode = msvc_solution::generation_mode::LOCAL;   
      
      if (opts.exists("target"))
         testing_target_name = opts["target"];

      BOOST_REQUIRE_NO_THROW(p_ = &engine_.load_project(test_data_path));
      BOOST_REQUIRE(p_);
      instantiate(test_data_path, testing_target_name);
      run_generators(test_data_path, mode);
      check(test_data_path);
   }

   jcf_parser checker_;
   auto_ptr<test_msvc_solution> msvc_solution_;
   const project* p_;
   vector<basic_target*> itargets_;
   vector<boost::intrusive_ptr<build_node> > nodes_;
   std::string test_name_;
   location_t generators_output_dir_name_;
};

static void test_function(const fs::path& test_data_path)
{
   generator_tests tests;
   tests.run_test(test_data_path);
}

void init_generators_tests(const fs::path& test_data_path)
{
   test_suite* ts = BOOST_TEST_SUITE("generators");
   for(fs::directory_iterator i(test_data_path / "generator_tests"); i != fs::directory_iterator(); ++i)
      if (i->path().filename() != ".svn")
         ts->add(make_test_case(boost::bind(&test_function, i->path()), i->path().filename().string()));

   framework::master_test_suite().add(ts);
}
