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

static const fs::path packages_filename("packages.json");

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
         dependency_attrs_p = public_id_p[assign_a(self.dependency_.public_id_, self.value_)] |
                              version_p[assign_a(self.dependency_.version_, self.value_)];
         // [push_back_a(self.package_.dependencies_, self.dependency_)][assign_a(self.dependency_, self.empty_dependency_)]
         dependency_p = confix_p(ch_p('{'), list_p(dependency_attrs_p, ch_p(',')), ch_p('}'));
         dependencies_p = str_p("dependencies") >> colon >> confix_p(ch_p('['), dependency_p, ch_p(']'));
         attribute_p = version_p[assign_a(self.package_.version_, self.value_)] |
                       filename_p[assign_a(self.package_.filename_, self.value_)] |
                       filesize_p |
                       public_id_p[assign_a(self.package_.public_id_, self.value_)] |
                       dependencies_p[push_back_a(self.package_.dependencies_, self.dependency_)][assign_a(self.dependency_, self.empty_dependency_)];
         package_p = confix_p(ch_p('{'), list_p(attribute_p, ch_p(',')), ch_p('}'));
         packages_p = list_p(package_p[push_back_a(self.packages_, self.package_)][assign_a(self.package_, self.empty_package_)], ch_p(','));

         whole = confix_p(ch_p('['), packages_p, ch_p(']'));

         BOOST_SPIRIT_DEBUG_RULE(whole);
         BOOST_SPIRIT_DEBUG_RULE(value_p);
         BOOST_SPIRIT_DEBUG_RULE(packages_p);
         BOOST_SPIRIT_DEBUG_RULE(package_p);
         BOOST_SPIRIT_DEBUG_RULE(attribute_p);
         BOOST_SPIRIT_DEBUG_RULE(package_name_p);
         BOOST_SPIRIT_DEBUG_RULE(dependencies_p);
         BOOST_SPIRIT_DEBUG_RULE(dependency_p);
         BOOST_SPIRIT_DEBUG_RULE(dependency_attrs_p);
      }

      static void assign_int_value(const gramma& self, unsigned int v) { self.int_value_ = v; }

      rule_t const& start() const { return whole; }
      rule_t whole, value_p, int_value_p, version_p, filename_p, filesize_p, public_id_p, attribute_p, package_p, packages_p, package_name_p,
             dependencies_p, dependency_p, dependency_attrs_p;
   };

   mutable string value_;
   mutable unsigned int int_value_;
   mutable vector<warehouse_impl::package_t> packages_;
   mutable warehouse_impl::package_t package_;
   mutable dependency_t dependency_;
   const warehouse_impl::package_t empty_package_;
   const dependency_t empty_dependency_;
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
                   const string& url,
                   const string& to_file = string())
{
   bp::context ctx;
   ctx.work_directory = working_dir.string();

   string cmd = "wget -c '" + url + "'";
   if (!to_file.empty())
      cmd += " -O " + to_file;

   bp::child child = bp::launch_shell(cmd, ctx);
   bp::status status = child.wait();
   if (status.exit_status() != 0)
      throw std::runtime_error("Failed to download '" + url + "'");
}

warehouse_impl::packages_t
warehouse_impl::load_packages(const fs::path& filepath)
{
   boost::filesystem::ifstream f(filepath);
   string packages_database_file;
   copy(istreambuf_iterator<char>(f), istreambuf_iterator<char>(), back_inserter(packages_database_file));

   gramma g;
   if (!parse(packages_database_file.begin(), packages_database_file.end(), g, +space_p).hit)
      throw std::runtime_error("Can't parse warehouse database");

   packages_t packages;
   for(vector<package_t>::const_iterator i = g.packages_.begin(), last = g.packages_.end(); i != last; ++i)
      packages.insert(make_pair(i->public_id_, *i));

   return packages;
}

void warehouse_impl::init_impl(const std::string& url)
{
   if (!exists(repository_path_)) {
      if (!create_directory(repository_path_))
         throw std::runtime_error("Failed to create directory '" + repository_path_.native() + "'");
   }

   const fs::path hamfile_path = repository_path_ / "hamfile";
   if (!exists(hamfile_path)) {
      fs::ofstream f(hamfile_path, ios_base::trunc);
      if (!f)
         throw std::runtime_error("Can't create '" + hamfile_path.string() + "'");
   }

   const fs::path hamroot_path = repository_path_ / "hamroot";
   if (!exists(hamroot_path)) {
      fs::ofstream f(hamroot_path, ios_base::trunc);
      if (!f)
         throw std::runtime_error("Can't create '" + hamroot_path.string() + "'");
   }

   const fs::path packages_full_filename = repository_path_ / packages_filename;
   if (!exists(packages_full_filename))
      download_file(repository_path_, url + "/" + packages_filename.string());

   packages_t new_packages = load_packages(packages_full_filename);

   engine_.load_project(repository_path_);

   repository_url_ = url;
   packages_.swap(new_packages);
}

static const string packages_update_filename = "packages.json.new";

void warehouse_impl::update_impl()
{
   if (repository_url_.empty())
      return;

   const fs::path packages_update_filepath = repository_path_ / packages_update_filename;
   const fs::path packages_filepath = repository_path_ / packages_filename;

   if (fs::exists(packages_update_filepath))
      fs::remove(packages_update_filepath);

   download_file(repository_path_, repository_url_ + "/" + packages_filename.string(), packages_update_filename);
   // check that file is OK
   load_packages(packages_update_filepath);

   fs::remove(packages_filepath);
   fs::rename(packages_update_filepath, packages_filepath);
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

warehouse_impl::packages_t::const_iterator
warehouse_impl::find_package(const std::string& public_id,
                             const std::string& version) const
{
   return const_cast<warehouse_impl*>(this)->find_package(public_id, version);
}

bool warehouse_impl::has_project(const location_t& project_path) const
{
   if (!project_path.has_root_path())
      return false;

   const string name = (++project_path.begin())->filename().string();

   return packages_.find(name) != packages_.end();
}

boost::shared_ptr<project>
warehouse_impl::load_project(const location_t& project_path)
{
   assert(has_project(project_path));

   const string name = (++project_path.begin())->filename().string();

   boost::shared_ptr<project> result(new warehouse_project(engine_, project_path));
   auto_ptr<basic_meta_target> target(new warehouse_meta_target(*result, pstring(engine_.pstring_pool(), name)));
   result->add_target(target);

   return result;
}

warehouse_impl::~warehouse_impl()
{

}

warehouse::package_info
warehouse_impl::to_package_info(const package_t& package)
{
   package_info p;

   p.name_ = package.public_id_;
   p.version_ = package.version_;
   p.package_file_size_ = package.filesize_;

   return p;
}

void warehouse_impl::resolve_dependency(unresolved_packages_t& packages,
                                        const dependency_t& d,
                                        const project& repository_project) const
{
   const string dep_hash = d.public_id_ + ":" + d.version_;
   unresolved_packages_t::const_iterator i = packages.find(dep_hash);
   if (i != packages.end())
      return;

   engine::loaded_projects_t loaded_projects = engine_.try_load_project("/" + d.public_id_, repository_project);
   feature_set* build_request = engine_.feature_registry().make_set();
   build_request->join("version", d.version_.c_str());
   project::selected_targets_t targets = loaded_projects.select_best_alternative(*build_request);
   if (targets.size() != 1)
      throw std::runtime_error("Failed to resolve dependency for package " + d.public_id_ + ":" + d.version_);

   if (!dynamic_cast<const warehouse_meta_target*>(targets.front().target_))
      return; // looks like package exists

   packages_t::const_iterator i_dependency_package = find_package(d.public_id_, d.version_);
   if (i_dependency_package == packages_.end())
      throw std::runtime_error("Package " + d.public_id_ + ":" + d.version_ + " doesn't exist");

   const package_t& package = i_dependency_package->second;
   packages.insert(make_pair(dep_hash, to_package_info(package)));

   for(vector<dependency_t>::const_iterator i = package.dependencies_.begin(), last = package.dependencies_.end(); i != last; ++i)
      resolve_dependency(packages, *i, repository_project);
}

vector<warehouse::package_info>
warehouse_impl::get_unresoved_targets_info(const std::vector<const warehouse_target*>& targets) const
{
   unresolved_dependencies_t deps;
   unresolved_packages_t packages;

   for(std::vector<const warehouse_target*>::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i) {
      package_info p;
      const feature_set& props = (**i).properties();
      feature_set::const_iterator version_i = props.find("version");
      if (version_i != props.end())
         p.version_ = (**version_i).value().to_string();
      else
         throw std::runtime_error("Unable to get unresolved target info - no version specified");

      p.name_ = (**i).name().to_string();
      packages_t::const_iterator pi = find_package(p.name_, p.version_);
      p.package_file_size_ = pi->second.filesize_;

      const package_t& package = pi->second;
      for(vector<dependency_t>::const_iterator i = package.dependencies_.begin(), last = package.dependencies_.end(); i != last; ++i) {
         const string dependency_hash = i->public_id_ + ":" + i->version_;
         if (packages.find(dependency_hash) == packages.end())
            deps.insert(make_pair(dependency_hash, *i));
      }

      const string package_hash = p.name_ + ":" + p.version_;
      packages.insert(make_pair(package_hash, p));
   }

   const project& repository_project = engine_.load_project(repository_path_);
   for(unresolved_dependencies_t::const_iterator i = deps.begin(), last = deps.end(); i != last; ++i)
      resolve_dependency(packages, i->second, repository_project);

   vector<package_info> result;
   for(unresolved_packages_t::const_iterator i = packages.begin(), last = packages.end(); i != last; ++i)
      result.push_back(i->second);

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
   fs::path tmp_name = filename.branch_path() / (filename.filename().string() + ".tmp");
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
   ctx.work_directory = package_root.native();
   fs::path package_archive = working_dir / "downloads" / p.filename_;
   bp::child child = bp::launch_shell("tar -xf '" + package_archive.string() + "'", ctx);
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

bool warehouse_impl::known_to_engine(const std::string& public_id,
                                     const project& repository_project)
{
   engine::loaded_projects_t loaded_projects = engine_.try_load_project("/" + public_id, repository_project);
   if (loaded_projects.empty())
      return false;

   const project& p = loaded_projects.front();
   return dynamic_cast<const warehouse_project*>(&p) == NULL;
}

void warehouse_impl::download_and_install(const std::vector<package_info>& packages)
{
   fs::path working_dir = repository_path_ / "downloads";
   if (!exists(working_dir))
      create_directory(working_dir);

   const project& repository_project = engine_.load_project(repository_path_);

   for(std::vector<package_info>::const_iterator i = packages.begin(), last = packages.end(); i != last; ++i) {
      packages_t::const_iterator pi = find_package(i->name_, i->version_);
      if (pi == packages_.end())
         throw std::runtime_error("Can't find package '" + i->name_ + " v" + i->version_ + "'");

      download_package(pi->second, working_dir);
      install_package(pi->second, repository_path_);

      if (!known_to_engine(pi->second.public_id_, repository_project)) {
         const fs::path repository_hamfile = repository_path_ / "hamfile";
         append_line(repository_hamfile, "use-project /" + pi->second.public_id_ + " : ./libs/" + pi->second.public_id_ + ";");
      }
   }
}

}
