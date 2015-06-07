#include <stdlib.h>
#include "warehouse_impl.h"
#include <hammer/core/engine.h>
//#define BOOST_SPIRIT_DEBUG
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include <boost/spirit/include/classic_chset.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/process.hpp>
#include <hammer/core/warehouse_project.h>
#include <hammer/core/warehouse_meta_target.h>
#include <hammer/core/warehouse_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <cassert>

using namespace std;
using namespace boost::spirit::classic;
namespace bp = boost::process;
namespace fs = boost::filesystem;

namespace hammer {

struct warehouse_impl::gramma : public grammar<warehouse_impl::gramma>
{
   template <typename ScannerT>
   struct definition
   {
      typedef rule<ScannerT> rule_t;

      definition(const gramma& self)
      {
         package_name_p = +chset_p("a-zA-Z0-9_.");

         chset<> colon(':');

         value_p = lexeme_d[confix_p(ch_p('"'), (*anychar_p)[assign_a(self.value_)], ch_p('"'))];
         int_value_p = confix_p(ch_p('"'), uint_p, ch_p('"'));
         version_p = str_p("version") >> colon >> value_p;
         filename_p = str_p("filename") >> colon >> value_p;
         filesize_p = str_p("filesize") >> colon >> int_value_p;
         public_id_p = str_p("public_id") >> colon >> value_p;

         attribute_p = version_p[assign_a(self.package_.version_, self.value_)] |
                       filename_p[assign_a(self.package_.filename_, self.value_)] |
                       filesize_p |
                       public_id_p[assign_a(self.package_.public_id_, self.value_)];
         package_p = (+package_name_p)[assign_a(self.package_.name_)] >> colon >> confix_p(ch_p('{'), list_p(attribute_p, ch_p(',')), ch_p('}'));
         packages_p = list_p(package_p[push_back_a(self.packages_, self.package_)][assign_a(self.package_, self.empty_package_)], ch_p(','));

         whole = confix_p(ch_p('{'), packages_p, ch_p('}'));

         BOOST_SPIRIT_DEBUG_RULE(whole);
         BOOST_SPIRIT_DEBUG_RULE(value_p);
         BOOST_SPIRIT_DEBUG_RULE(packages_p);
         BOOST_SPIRIT_DEBUG_RULE(package_p);
         BOOST_SPIRIT_DEBUG_RULE(attribute_p);
         BOOST_SPIRIT_DEBUG_RULE(package_name_p);
      }

      static void assign_int_value(const gramma& self, unsigned int v) { self.int_value_ = v; }

      rule_t const& start() const { return whole; }
      rule_t whole, value_p, int_value_p, version_p, filename_p, filesize_p, public_id_p, attribute_p, package_p, packages_p, package_name_p;
   };

   mutable string value_;
   mutable unsigned int int_value_;
   mutable vector<warehouse_impl::package_t> packages_;
   mutable warehouse_impl::package_t package_;
   const warehouse_impl::package_t empty_package_;
};

static
fs::path get_home_path()
{
#if defined(_WIN32)

   const char* home_path = getenv("USERPROFILE");
   if (home_path != NULL)
      return hammer::location_t(home_path);
   else
      throw std::runtime_error("Can't find user home directory.");

#else
#   if defined(__linux__)

   const char* home_path = getenv("HOME");
   if (home_path != NULL)
      return hammer::location_t(home_path);
   else
      throw std::runtime_error("Can't find user home directory.");

#   else
#      error "Platform not supported"
#   endif
#endif

}

warehouse_impl::warehouse_impl(engine& e)
   : engine_(e),
     repository_path_(get_home_path() / ".hammer")
{
}

static
void download_file(const fs::path& working_dir,
                   const string& url)
{
   bp::context ctx;
   ctx.work_directory = working_dir.native_directory_string();
   bp::child child = bp::launch_shell("wget -c '" + url + "'", ctx);
   bp::status status = child.wait();
   if (status.exit_status() != 0)
      throw std::runtime_error("Failed to download '" + url + "'");
}

void warehouse_impl::init_impl(const std::string& url)
{
   if (!exists(repository_path_)) {
      if (!create_directory(repository_path_))
         throw std::runtime_error("Failed to create directory '" + repository_path_.native_directory_string() + "'");
   }

   const fs::path hamfile_path = repository_path_ / "hamfile";
   if (!exists(hamfile_path)) {
      fs::ofstream f(hamfile_path, ios_base::trunc);
      if (!f)
         throw std::runtime_error("Can't create '" + hamfile_path.native_file_string() + "'");
   }

   const fs::path hamroot_path = repository_path_ / "hamroot";
   if (!exists(hamroot_path)) {
      fs::ofstream f(hamroot_path, ios_base::trunc);
      if (!f)
         throw std::runtime_error("Can't create '" + hamroot_path.native_file_string() + "'");
   }

   const fs::path packages_filename = repository_path_ / "packages.json";
   if (!exists(packages_filename))
      download_file(repository_path_, url + "/packages.json");

   boost::filesystem::ifstream f(repository_path_ / "packages.json");
   string packages_database_file;
   copy(istreambuf_iterator<char>(f), istreambuf_iterator<char>(), back_inserter(packages_database_file));

   gramma g;
   if (!parse(packages_database_file.begin(), packages_database_file.end(), g, +space_p).hit)
      throw std::runtime_error("Can't parse warehouse database");

   packages_t new_packages;
   for(vector<package_t>::const_iterator i = g.packages_.begin(), last = g.packages_.end(); i != last; ++i)
      new_packages.insert(make_pair(i->public_id_, *i));

   engine_.load_project(repository_path_);

   repository_url_ = url;
   packages_.swap(new_packages);
}

warehouse_impl::packages_t::iterator
warehouse_impl::find_package(const std::string& public_id,
                             const std::string& version)
{
   pair<packages_t::iterator, packages_t::iterator> versions = packages_.equal_range(public_id);
   for(; versions.first != versions.second; ++versions.first)
      if (versions.first->second.version_ == version)
         return versions.first;

   return packages_.end();
}

bool warehouse_impl::has_project(const location_t& project_path) const
{
   if (!project_path.has_root_path())
      return false;

   const string name = *++project_path.begin();

   return packages_.find(name) != packages_.end();
}

boost::shared_ptr<project>
warehouse_impl::load_project(const location_t& project_path)
{
   assert(has_project(project_path));

   const string name = *++project_path.begin();

   boost::shared_ptr<project> result(new warehouse_project(engine_, project_path));
   auto_ptr<basic_meta_target> target(new warehouse_meta_target(*result, pstring(engine_.pstring_pool(), name)));
   result->add_target(target);

   return result;
}

warehouse_impl::~warehouse_impl()
{

}

vector<warehouse::package_info>
warehouse_impl::get_unresoved_targets_info(const std::vector<const warehouse_target*>& targets) const
{
   vector<warehouse::package_info> result;
   for(std::vector<const warehouse_target*>::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i) {
      package_info p;
      packages_t::const_iterator pi = packages_.find((**i).name().to_string());
      if (pi == packages_.end()) {
         p.name_ = "unknown";
         p.package_file_size_ = 0;
         p.version_ = "unknown";
      } else {
         p.name_ = pi->first;
         p.package_file_size_ = pi->second.filesize_;
         const feature_set& props = (**i).properties();
         feature_set::const_iterator version_i = props.find("version");
         if (version_i != props.end())
            p.version_ = (**version_i).value().to_string();
         else
            p.version_ = "unknown";
      }

      result.push_back(p);
   }

   return result;
}

void warehouse_impl::download_package(const package_t& p,
                                      const fs::path& working_dir)
{
   const string package_url = repository_url_ + "/" + p.filename_;
   download_file(working_dir, package_url);
}

static
void insert_line_in_front(const fs::path& filename,
                          const string& line)
{
   fs::path tmp_name = filename.branch_path() / (filename.filename() + ".tmp");
   {
      fs::ifstream src(filename);
      fs::ofstream tmp_file(tmp_name);
      tmp_file << line << endl;
      copy(istreambuf_iterator<char>(src), istreambuf_iterator<char>(), ostreambuf_iterator<char>(tmp_file));
   }

   fs::remove(filename);
   fs::rename(tmp_name, filename);
}

static
void append_line(const fs::path filename,
                 const string& line)
{
   fs::ofstream f(filename, ios_base::app);
   f << line << endl;
}

void warehouse_impl::install_package(const package_t& p,
                                     const fs::path& working_dir)
{
   const fs::path libs = working_dir / "libs";
   if (!exists(libs))
      create_directory(libs);

   const fs::path package_root = libs / p.public_id_;
   if (!exists(package_root))
      create_directory(package_root);

   bp::context ctx;
   ctx.work_directory = package_root.native_directory_string();
   fs::path package_archive = working_dir / "downloads" / p.filename_;
   bp::child child = bp::launch_shell("tar -xf '" + package_archive.native_file_string() + "'", ctx);
   bp::status status = child.wait();
   if (status.exit_status() != 0)
      throw std::runtime_error("Failed to unpack package '" + p.public_id_ + "'");

   fs::path package_hamfile = package_root / "hamfile";
   if (!exists(package_hamfile)) {
      fs::ofstream f(package_hamfile);
      f << "warehouse-trap " << p.public_id_ << ";";
      f.close();
   }

   insert_line_in_front(package_hamfile, "version-alias " + p.public_id_ + " : " + p.version_ + ";");
}

void warehouse_impl::download_and_install(const std::vector<package_info>& packages)
{
   fs::path working_dir = repository_path_ / "downloads";
   if (!exists(working_dir))
      create_directory(working_dir);

   for(std::vector<package_info>::const_iterator i = packages.begin(), last = packages.end(); i != last; ++i) {
      packages_t::const_iterator pi = find_package(i->name_, i->version_);
      if (pi == packages_.end())
         throw std::runtime_error("Can't find package '" + i->name_ + " v" + i->version_ + "'");

      download_package(pi->second, working_dir);
      install_package(pi->second, repository_path_);

      fs::path repository_hamfile = repository_path_ / "hamfile";
      if (!exists(repository_hamfile)) {
         fs::ofstream f(repository_hamfile);
         f.close();
      }

      append_line(repository_hamfile, "use-project /" + pi->second.public_id_ + " : ./libs/" + pi->second.public_id_ + ";");
   }
}

}
