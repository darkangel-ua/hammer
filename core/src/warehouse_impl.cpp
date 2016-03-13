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
#include <boost/foreach.hpp>
#include <boost/algorithm/string/replace.hpp>
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
   gramma() {}

   template <typename ScannerT>
   struct definition
   {
      typedef rule<ScannerT> rule_t;

      definition(const gramma& self)
      {
         package_name_p = +chset_p("a-zA-Z0-9_.");

         chset<> colon(':');

         value_p = lexeme_d[confix_p(ch_p('"'), (*anychar_p)[assign_a(self.value_)], ch_p('"'))];
         int_value_impl_p = uint_p[boost::bind(&gramma::assign_int_value, &self, _1)];
         int_value_p = confix_p(ch_p('"'), int_value_impl_p, ch_p('"'));
         version_p = str_p("version") >> colon >> value_p;
         filename_p = str_p("filename") >> colon >> value_p;
         filesize_p = str_p("filesize") >> colon >> int_value_p;
         public_id_p = str_p("public_id") >> colon >> value_p;
         dependency_attrs_p = public_id_p[assign_a(self.dependency_.public_id_, self.value_)] |
                              version_p[assign_a(self.dependency_.version_, self.value_)];
         // [push_back_a(self.package_.dependencies_, self.dependency_)][assign_a(self.dependency_, self.empty_dependency_)]
         dependencies_list_p = list_p(dependency_attrs_p, ch_p(','));
         dependency_p = confix_p(ch_p('{'), dependencies_list_p[push_back_a(self.package_.dependencies_, self.dependency_)][assign_a(self.dependency_, self.empty_dependency_)], ch_p('}'));
         dependencies_p = str_p("dependencies") >> colon >> confix_p(ch_p('['), list_p(dependency_p, ch_p(',')), ch_p(']'));
         attribute_p = version_p[assign_a(self.package_.version_, self.value_)] |
                       filename_p[assign_a(self.package_.filename_, self.value_)] |
                       filesize_p[assign_a(self.package_.filesize_, self.int_value_)] |
                       public_id_p[assign_a(self.package_.public_id_, self.value_)] |
                       dependencies_p;
         package_p = confix_p(ch_p('{'), list_p(attribute_p, ch_p(',')), ch_p('}'));
         packages_p = list_p(package_p[push_back_a(self.packages_, self.package_)][assign_a(self.package_, self.empty_package_)], ch_p(','));

         whole = confix_p(ch_p('['), !packages_p, ch_p(']'));

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

      rule_t const& start() const { return whole; }
      rule_t whole, value_p, int_value_p, int_value_impl_p, version_p, filename_p, filesize_p, public_id_p, attribute_p, package_p, packages_p, package_name_p,
             dependencies_p, dependency_p, dependency_attrs_p, dependencies_list_p;
   };

   mutable string value_;
   mutable unsigned int int_value_;
   mutable vector<warehouse_impl::package_t> packages_;
   mutable warehouse_impl::package_t package_;
   mutable dependency_t dependency_;
   const warehouse_impl::package_t empty_package_;
   const dependency_t empty_dependency_;

   void assign_int_value(unsigned v) const { int_value_ = v; }
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
   if (url.find_first_of("file://") == 0) {
      fs::path source_file = url.substr(7);
      fs::path destination_file = to_file.empty() ? (working_dir / source_file.filename()) : (working_dir / to_file);
      fs::copy_file(source_file, destination_file);
      return;
   }

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

void warehouse_impl::init_impl(const std::string& url,
                               const string& storage_dir)
{
   if (!storage_dir.empty())
      repository_path_ = storage_dir;

   if (!exists(repository_path_)) {
      if (!create_directory(repository_path_))
         throw std::runtime_error("Failed to create directory '" + repository_path_.string() + "'");
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
warehouse_impl::find_package(packages_t& packages,
                             const std::string& public_id,
                             const std::string& version)
{
   pair<packages_t::iterator, packages_t::iterator> versions = packages.equal_range(public_id);
   for(; versions.first != versions.second; ++versions.first)
      if (versions.first->second.version_ == version)
         return versions.first;

   return packages.end();
}

warehouse_impl::packages_t::iterator
warehouse_impl::find_package(const std::string& public_id,
                             const std::string& version)
{
   return find_package(packages_, public_id, version);
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

   const string name = project_path.relative_path().string();

   return packages_.find(name) != packages_.end();
}

boost::shared_ptr<project>
warehouse_impl::load_project(const location_t& project_path)
{
   assert(has_project(project_path));

   const string name = project_path.relative_path().string();

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
   const fs::path libs_path = working_dir / "libs";
   if (!exists(libs_path))
      create_directory(libs_path);

   const fs::path lib_path = libs_path / p.public_id_;
   if (!exists(lib_path))
      create_directories(lib_path);

   const fs::path package_root = lib_path / p.version_;
   if (!exists(package_root))
      create_directory(package_root);

   bp::context ctx;
   ctx.work_directory = package_root.string();
   const fs::path package_archive = working_dir / "downloads" / p.filename_;
   bp::child child = bp::launch_shell("tar -xf '" + package_archive.string() + "'", ctx);
   bp::status status = child.wait();
   if (status.exit_status() != 0)
      throw std::runtime_error("Failed to unpack package '" + p.public_id_ + "'");

   const fs::path package_hamfile = lib_path / "hamfile";
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
         const fs::path repository_hamroot = repository_path_ / "hamroot";
         append_line(repository_hamroot, "use-project /" + pi->second.public_id_ + " : ./libs/" + pi->second.public_id_ + ";");
      }
   }
}

void warehouse_impl::write_packages(const location_t& packages_db_path,
                                    const packages_t& packages)
{
   const fs::path tmp_packages_path = fs::unique_path();
   fs::ofstream tmp_packages(tmp_packages_path);
   tmp_packages << "[\n";
   bool first = true;
   for(packages_t::const_iterator i = packages.begin(), last = packages.end(); i != last; ++i) {
      if (first) {
         tmp_packages << "   {\n";
         first = false;
      } else
         tmp_packages << "  ,{\n";
      tmp_packages << "       public_id : \"" << i->second.public_id_ << "\",\n";
      tmp_packages << "       version   : \"" << i->second.version_ << "\",\n";
      tmp_packages << "       filename  : \"" << i->second.filename_ << "\",\n";
      tmp_packages << "       filesize  : \"" << i->second.filesize_ << "\"";
      if (i->second.dependencies_.empty())
         tmp_packages << "\n";
      else {
         tmp_packages << ",\n       dependencies : [\n";
         bool d_first = true;
         for(vector<dependency_t>::const_iterator d = i->second.dependencies_.begin(), d_last = i->second.dependencies_.end(); d != d_last; ++d) {
            if (d_first) {
               tmp_packages << "          {\n";
               d_first = false;
            } else
               tmp_packages << "         ,{\n";

            tmp_packages << "            public_id : \"" << d->public_id_ << "\",\n";
            tmp_packages << "            version   : \"" << d->version_ << "\"\n";
            tmp_packages << "          }\n";
         }

         tmp_packages << "       ]\n";
      }

      tmp_packages << "   }\n";
   }

   tmp_packages << "]\n";
   tmp_packages.close();

   if (fs::remove(packages_db_path)) {
      try {
         fs::rename(tmp_packages_path, packages_db_path);
      } catch(const std::exception&) {
         fs::copy_file(tmp_packages_path, packages_db_path);
         fs::remove(tmp_packages_path);
      }
   } else
      throw std::runtime_error("Can't remove old packages db");
}

static
void make_package_archive(const fs::path& package_root,
                          const fs::path& package_filename)
{
   bp::context ctx;
   ctx.work_directory = package_root.string();
   bp::child child = bp::launch_shell("tar --exclude \"\\.hammer\" --exclude-vcs -cjf '" + package_filename.string() + "' .", ctx);
   bp::status status = child.wait();
   if (status.exit_status() != 0)
      throw std::runtime_error("Failed to create package archive");
}

vector<warehouse_impl::dependency_t>
warehouse_impl::gather_dependencies(const project& p)
{
   vector<source_decl> source_dependencies;
   for(project::targets_t::const_iterator i = p.targets().begin(), last = p.targets().end(); i != last; ++i) {
      const sources_decl& sources = i->second->sources();
      for(sources_decl::const_iterator s_i = sources.begin(), s_last = sources.end(); s_i != s_last; ++s_i) {
         if (s_i->target_path_is_global())
            source_dependencies.push_back(*s_i);
      }
   }

   sort(source_dependencies.begin(), source_dependencies.end());
   source_dependencies.erase(unique(source_dependencies.begin(), source_dependencies.end()), source_dependencies.end());

   vector<dependency_t> dependencies;
   BOOST_FOREACH(const source_decl& s, source_dependencies) {
      feature_set::const_iterator i = s.properties()->find("version");
      if (i == s.properties()->end())
         throw std::runtime_error("Dependency '" + s.target_path().to_string() + "' doesn't have version specified");

      dependency_t d;
      d.public_id_ = string(s.target_path().begin() + 1, s.target_path().end());
      d.version_ = (**i).value().to_string();

      dependencies.push_back(d);
   }

   return dependencies;
}

void warehouse_impl::add_to_packages(const project& p,
                                     const location_t& packages_db_root)
{
   fs::path packages_db_full_path = packages_db_root / packages_filename;
   if (!exists(packages_db_full_path)) {
      fs::ofstream f(packages_db_full_path, ios_base::trunc);
      if (!f)
         throw std::runtime_error("Can't create '" + packages_db_full_path.string() + "'");
      f << "[]";
   }

   packages_t packages = load_packages(packages_db_full_path);
   const feature_set* build_request = p.get_engine()->feature_registry().make_set();
   feature_set* project_requirements  = p.get_engine()->feature_registry().make_set();
   p.requirements().eval(*build_request, project_requirements);
   feature_set::const_iterator i_version = project_requirements->find("version");
   if (i_version == project_requirements->end())
      throw std::runtime_error("Project doesn't have 'version' feature");

   const string version = (**i_version).value().to_string();
   const string public_id = p.name().to_string();

   package_t package;
   package.public_id_ = public_id;
   package.version_ = version;
   package.filename_ = boost::algorithm::replace_all_copy(public_id, "/", ".") + "-" + version + ".tar.bz2";

   const fs::path package_full_path = packages_db_root / package.filename_;
   make_package_archive(p.location().branch_path().branch_path(), package_full_path);

   package.filesize_ = file_size(package_full_path);
   package.dependencies_ = gather_dependencies(p);

   packages_t::iterator i = find_package(packages, public_id, version);
   if (i == packages.end())
      packages.insert(make_pair(public_id, package));
   else
      i->second = package;

   write_packages(packages_db_full_path, packages);
}

}
