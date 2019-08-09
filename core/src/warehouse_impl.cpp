#include <stdlib.h>
#include <cassert>
#include <unordered_set>
#include <unordered_map>
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
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/crypto/md5.hpp>
#include <boost/make_unique.hpp>
#include <boost/regex.hpp>
#include <hammer/core/warehouse_impl.h>
#include <hammer/core/engine.h>
#include <hammer/core/warehouse_project.h>
#include <hammer/core/warehouse_meta_target.h>
#include <hammer/core/warehouse_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/virtual_project.h>
#include <hammer/core/build_request.h>

using namespace std;
using namespace boost::spirit::classic;
namespace bp = boost::process;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

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
         int_value_p = uint_p[boost::bind(&gramma::assign_int_value, &self, _1)];
         bool_value_p = str_p("true")[boost::bind(&gramma::assign_bool_value, &self, true)] |
                        str_p("false")[boost::bind(&gramma::assign_bool_value, &self, false)];
         version_p = str_p("version") >> colon >> value_p;
         filename_p = str_p("filename") >> colon >> value_p;
         filesize_p = str_p("filesize") >> colon >> int_value_p;
         md5_p = str_p("md5") >> colon >> value_p;
         public_id_p = str_p("public_id") >> colon >> value_p;
         need_update_p = str_p("need_update") >> colon >> bool_value_p;
         list_of_target_names_p = list_p(value_p[push_back_a(self.targets_, self.value_)], ch_p(','));
         targets_p = str_p("targets") >> colon >> confix_p(ch_p('['), list_of_target_names_p, ch_p(']'));
         dependency_attrs_p = public_id_p[assign_a(self.dependency_.public_id_, self.value_)] |
                              version_p[assign_a(self.dependency_.version_, self.value_)];
         dependencies_list_p = list_p(dependency_attrs_p, ch_p(','));
         dependency_p = confix_p(ch_p('{'), dependencies_list_p[push_back_a(self.package_.dependencies_, self.dependency_)][assign_a(self.dependency_, self.empty_dependency_)], ch_p('}'));
         dependencies_p = str_p("dependencies") >> colon >> confix_p(ch_p('['), list_p(dependency_p, ch_p(',')), ch_p(']'));
         attribute_p = version_p[assign_a(self.package_.version_, self.value_)] |
                       filename_p[assign_a(self.package_.filename_, self.value_)] |
                       md5_p[assign_a(self.package_.md5_, self.value_)] |
                       filesize_p[assign_a(self.package_.filesize_, self.int_value_)] |
                       public_id_p[assign_a(self.package_.public_id_, self.value_)] |
                       need_update_p[assign_a(self.package_.need_update_, self.bool_value_)] |
                       targets_p[assign_a(self.package_.targets_, self.targets_)][assign_a(self.targets_, self.empty_targets_)] |
                       dependencies_p;
         package_p = confix_p(ch_p('{'), list_p(attribute_p, ch_p(',')), ch_p('}'));
         packages_p = list_p(package_p[push_back_a(self.packages_, self.package_)][assign_a(self.package_, self.empty_package_)], ch_p(','));

         whole = confix_p(ch_p('['), !packages_p, ch_p(']'));

         BOOST_SPIRIT_DEBUG_RULE(whole);
         BOOST_SPIRIT_DEBUG_RULE(value_p);
         BOOST_SPIRIT_DEBUG_RULE(bool_value_p);
         BOOST_SPIRIT_DEBUG_RULE(packages_p);
         BOOST_SPIRIT_DEBUG_RULE(package_p);
         BOOST_SPIRIT_DEBUG_RULE(attribute_p);
         BOOST_SPIRIT_DEBUG_RULE(package_name_p);
         BOOST_SPIRIT_DEBUG_RULE(need_update_p);
         BOOST_SPIRIT_DEBUG_RULE(targets_p);
         BOOST_SPIRIT_DEBUG_RULE(dependencies_p);
         BOOST_SPIRIT_DEBUG_RULE(dependency_p);
         BOOST_SPIRIT_DEBUG_RULE(dependency_attrs_p);
      }

      rule_t const& start() const { return whole; }
      rule_t whole, value_p, int_value_p, bool_value_p, version_p, filename_p, filesize_p, public_id_p, md5_p, need_update_p, attribute_p, package_p, packages_p, package_name_p,
             list_of_target_names_p, targets_p, dependencies_p, dependency_p, dependency_attrs_p, dependencies_list_p;
   };

   mutable string value_;
   mutable unsigned int int_value_;
   mutable bool bool_value_;
   mutable vector<warehouse_impl::package_t> packages_;
   mutable warehouse_impl::package_t package_;
   mutable vector<string> targets_;
   mutable dependency_t dependency_;
   const warehouse_impl::package_t empty_package_;
   const dependency_t empty_dependency_;
   const vector<string> empty_targets_;

   void assign_int_value(unsigned v) const { int_value_ = v; }
   void assign_bool_value(bool v) const { bool_value_ = v; }
};

static
fs::path get_home_path()
{
#if defined(_WIN32)

   const char* home_path = getenv("USERPROFILE");
   if (home_path)
      return hammer::location_t(home_path);
   else
      throw std::runtime_error("Can't find user home directory.");

#else
#   if defined(__linux__)

   const char* home_path = getenv("HOME");
   if (home_path)
      return hammer::location_t(home_path);
   else
      throw std::runtime_error("Can't find user home directory.");

#   else
#      error "Platform not supported"
#   endif
#endif

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
//   ctx.stderr_behavior = bp::inherit_stream();
//   ctx.stdout_behavior = bp::inherit_stream();
   ctx.work_directory = working_dir.string();

   string cmd;
   if (to_file.empty())
       cmd = "curl -L -O " + url + "";
   else
       cmd = "curl -L -o " + to_file + " " + url;

   bp::child child = bp::launch_shell(cmd, ctx);
   bp::status status = child.wait();
   if (status.exit_status() != 0)
      throw std::runtime_error("Failed to download '" + url + "'");
}

class trap_wh_project : public virtual_project {
   public:
      trap_wh_project(engine& e,
                      const project* parent,
                      const std::string& name)
         : virtual_project(e, parent, name)
      {}
};

class global_trap_wh_project : public virtual_project {
   public:
      global_trap_wh_project(engine& e,
                             warehouse_impl& wh)
         : virtual_project(e, nullptr),
           warehouse_(wh)
      {}

      loaded_projects
      load_project(const location_t& path) const override;
      void reset_traps() { traps_.clear(); }

   private:
      using traps = boost::unordered_map<location_t, std::unique_ptr<project>>;
      mutable traps traps_;
      warehouse_impl& warehouse_;
};

loaded_projects
global_trap_wh_project::load_project(const location_t& path) const
{
   if (!warehouse_.has_project(path, {}))
      return {};

   // if it's already materialized we already installed traps so we don't need to do anything
   if (warehouse_.already_materialized(path))
      return {};

   // aha, it doesn't - we need to create one, that contains traps
   auto i = traps_.find(path);
   if (i != traps_.end())
      return loaded_projects{i->second.get()};
   else {
      auto trap_project = boost::make_unique<trap_wh_project>(get_engine(), this, path.string());
      project* raw_trap_project = trap_project.get();
      add_traps(warehouse_, *trap_project, path.string());
      traps_.insert({path, std::move(trap_project)}) ;

      return loaded_projects{raw_trap_project};
   }
}

static
fs::path
make_storage_dir(const boost::filesystem::path& storage_dir) {
   fs::path result = storage_dir.empty() ? (get_home_path() / ".hammer") : storage_dir;
   // path should be without trailing '.' to be correctly compared in package_from_warehouse()
   result.normalize();
   if (result.filename() == ".")
      result.remove_leaf();

   return result;
}

warehouse_impl::warehouse_impl(engine& e,
                               const std::string& id,
                               const std::string& url,
                               const boost::filesystem::path& storage_dir)
   : warehouse(id, make_storage_dir(storage_dir)),
     repository_url_(url)
{
   if (!storage_dir_.has_root_path())
      throw std::runtime_error("Warehouse storage directory should be a full path");

   if (!exists(storage_dir_)) {
      if (!create_directory(storage_dir_))
         throw std::runtime_error("Failed to create directory '" + storage_dir_.string() + "'");
   }

   const fs::path hamroot_path = storage_dir_ / "hamroot";
   if (!exists(hamroot_path)) {
      fs::ofstream f(hamroot_path, ios_base::trunc);
      if (!f)
         throw std::runtime_error("Can't create '" + hamroot_path.string() + "'");
   }

   const fs::path packages_full_filename = storage_dir_ / packages_filename;
   if (!exists(packages_full_filename))
      download_file(storage_dir_, url + "/" + packages_filename.string());

   packages_ = load_packages(packages_full_filename);

   // adding global alias on our storage dir and it will be automaticaly loaded
   // on engine.load_project for global projects
   e.add_alias(location_t{"/"}, storage_dir_, nullptr);

   auto global_trap_project = boost::make_unique<global_trap_wh_project>(e, *this);
   global_trap_project_ = global_trap_project.get();
   e.insert(std::move(global_trap_project));
   e.add_alias(location_t{"/"}, global_trap_project_->location(), nullptr);
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

static const string packages_update_filename = "packages.json.new";

warehouse::package_infos_t
warehouse_impl::update_impl()
{
   if (repository_url_.empty())
      return package_infos_t();

   const fs::path packages_update_filepath = storage_dir_ / packages_update_filename;
   const fs::path packages_filepath = storage_dir_ / packages_filename;

   if (fs::exists(packages_update_filepath))
      fs::remove(packages_update_filepath);

   download_file(storage_dir_, repository_url_ + "/" + packages_filename.string(), packages_update_filename);

   packages_t new_packages = load_packages(packages_update_filepath);
   package_infos_t packages_needs_to_be_updated;

   for(auto& pv : new_packages) {
      package_t& p = pv.second;
      auto i = find_package(p.public_id_, p.version_);
      if (i != packages_.end() && p.md5_ != i->second.md5_) {
         p.need_update_ = true;

         package_info package_to_update;
         package_to_update.name_ = p.public_id_;
         package_to_update.version_ = p.version_;
         package_to_update.package_file_size_ = p.filesize_;

         packages_needs_to_be_updated.push_back(package_to_update);
      }
   }

   write_packages(packages_update_filepath, new_packages);

   fs::remove(packages_filepath);
   fs::rename(packages_update_filepath, packages_filepath);

   return packages_needs_to_be_updated;
}

void warehouse_impl::update_all_packages_impl(engine& e)
{
   for(auto& p : packages_) {
      if (p.second.need_update_)
         update_package(e, p.second);
   }

   write_packages(storage_dir_ / packages_filename, packages_);
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

bool warehouse_impl::owned(const project& p) const
{
   if (dynamic_cast<const trap_wh_project*>(&p))
      return true;

   auto project_location = p.location();
   if (project_location.filename() == ".")
      project_location.remove_leaf();

   return project_location.string().find(storage_dir_.string().c_str()) == 0;
}

bool warehouse_impl::has_project(const location_t& project_path,
                                 const string& version) const
{
   assert(!project_path.has_root_directory());

   const string name = project_path.string();

   auto packages = packages_.equal_range(name);
   if (packages.first == packages.second)
      return false;

   if (version.empty())
      return packages.first != packages_.end();

   for(; packages.first != packages.second; ++packages.first)
      if (packages.first->second.version_ == version)
         return true;

   return false;
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
                                        engine& e,
                                        const dependency_t& d,
                                        const project& repository_project) const
{
   const string dep_hash = d.public_id_ + ":" + d.version_;
   unresolved_packages_t::const_iterator i = packages.find(dep_hash);
   if (i != packages.end())
      return;

   loaded_projects loaded_projects = e.load_project(engine::global_project_ref{"/" + d.public_id_});
   feature_set* build_request = e.feature_registry().make_set();
   build_request->join("version", d.version_.c_str());
   project::selected_targets_t targets = loaded_projects.select_best_alternative(hammer::build_request{*build_request});
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
      resolve_dependency(packages, e, *i, repository_project);
}

vector<warehouse::package_info>
warehouse_impl::get_unresoved_targets_info(engine& e,
                                           const std::vector<const warehouse_target*>& targets) const
{
   unresolved_dependencies_t deps;
   unresolved_packages_t packages;

   for(std::vector<const warehouse_target*>::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i) {
      package_info p;
      const feature_set& props = (**i).properties();
      feature_set::const_iterator version_i = props.find("version");
      if (version_i != props.end())
         p.version_ = (**version_i).value();
      else
         throw std::runtime_error("Unable to get unresolved target info - no version specified");

      p.name_ = (**i).get_project().name();

      const string package_hash = p.name_ + ":" + p.version_;
      // check for duplicates
      if (packages.find(package_hash) != packages.end())
         continue;

      packages_t::const_iterator pi = find_package(p.name_, p.version_);
      p.package_file_size_ = pi->second.filesize_;

      const package_t& package = pi->second;
      for(vector<dependency_t>::const_iterator i = package.dependencies_.begin(), last = package.dependencies_.end(); i != last; ++i) {
         const string dependency_hash = i->public_id_ + ":" + i->version_;
         if (packages.find(dependency_hash) == packages.end())
            deps.insert(make_pair(dependency_hash, *i));
      }

      packages.insert(make_pair(package_hash, p));
   }

   const project& repository_project = e.load_project(storage_dir_);
   for(unresolved_dependencies_t::const_iterator i = deps.begin(), last = deps.end(); i != last; ++i)
      resolve_dependency(packages, e, i->second, repository_project);

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
void append_line(const fs::path& filename,
                 const string& line)
{
   fs::ofstream f(filename, ios_base::app);
   f << line << endl;
}

static
void add_new_target(const fs::path& filename,
                    const string& new_target_line)
{
   const fs::path tmp_name = filename.branch_path() / (filename.filename().string() + ".tmp");
   {
      fs::ifstream src(filename);
      fs::ofstream tmp_file(tmp_name);
      string line;
      while (getline(src, line)) {
         if (line.find("warehouse-trap") == 0) {
            tmp_file << new_target_line << endl
                     << line;
            break;
         } else
            tmp_file << line << endl;
      }
   }

   fs::remove(filename);
   fs::rename(tmp_name, filename);
}

static
string make_package_proxy_line(const string& package_version)
{
   return "use-project location = ./" + package_version + " : requirements = <version>" + package_version + ";";
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
   bp::child child = bp::launch_shell("bzcat " + package_archive.string() + " | tar -x", ctx);
   bp::status status = child.wait();
   if (status.exit_status() != 0)
      throw std::runtime_error("Failed to unpack package '" + p.public_id_ + "'");

   const fs::path package_hamfile = lib_path / "hamfile";
   if (!exists(package_hamfile)) {
      fs::ofstream f(package_hamfile);
      f << "project " << p.public_id_ << ";\n\n";
      f << "warehouse-trap \"" << p.public_id_ << "\";\n";
      f.close();
   }

   add_new_target(package_hamfile, make_package_proxy_line(p.version_));
}

void warehouse_impl::reload(engine& e) {
   e.unload_project(e.load_project(storage_dir_));

   static_cast<global_trap_wh_project*>(global_trap_project_)->reset_traps();
}

void warehouse_impl::download_and_install(engine& e,
                                          const std::vector<package_info>& packages,
                                          iwarehouse_download_and_install& notifier)
{
   fs::path working_dir = storage_dir_ / "downloads";
   if (!exists(working_dir))
      create_directory(working_dir);

   size_t index = 0;
   for(std::vector<package_info>::const_iterator i = packages.begin(), last = packages.end(); i != last; ++i, ++index) {
      packages_t::const_iterator pi = find_package(i->name_, i->version_);
      if (pi == packages_.end())
         throw std::runtime_error("Can't find package '" + i->name_ + " v" + i->version_ + "'");

      const package_info bpi = to_package_info(pi->second);
      if (!notifier.on_download_begin(index, bpi))
         return;

      download_package(pi->second, working_dir);
      notifier.on_download_end(index, bpi);

      if (!notifier.on_install_begin(index, bpi))
         return;

      const bool project_already_materilized = fs::exists(storage_dir_ / "libs" / pi->second.public_id_);
      install_package(pi->second, storage_dir_);
      if (!project_already_materilized) {
         const fs::path repository_hamroot = storage_dir_ / "hamroot";
         append_line(repository_hamroot, "use-project " + pi->second.public_id_ + " : ./libs/" + pi->second.public_id_ + ";");
      }

      notifier.on_install_end(index, bpi);
   }

   reload(e);
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
      tmp_packages << "       public_id  : \"" << i->second.public_id_ << "\",\n";
      tmp_packages << "       version    : \"" << i->second.version_ << "\",\n";
      tmp_packages << "       filename   : \"" << i->second.filename_ << "\",\n";
      tmp_packages << "       md5        : \"" << i->second.md5_ << "\",\n";
      tmp_packages << "       need_update: " << boolalpha << i->second.need_update_ << ",\n";
      tmp_packages << "       filesize   : " << i->second.filesize_;
      if (!i->second.targets_.empty()) {
         tmp_packages << ",\n       targets    : [ ";
         bool first = true;
         for (const string& t : i->second.targets_) {
            if (first)
               first = false;
            else
               tmp_packages << ", ";

            tmp_packages << '"' << t << '"';
         }

         tmp_packages << " ]";
      }

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
   ctx.stdout_behavior = bp::inherit_stream();
   bp::child child = bp::launch_shell("tar --exclude \\.git --exclude \\.svn --exclude \\.hammer -c . | bzip2 -c > " + package_filename.string(), ctx);
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
   for(const source_decl& s : source_dependencies) {
      auto& build_request = s.build_request()->resolved_request();
      feature_set::const_iterator i = build_request.find("version");
      if (i == build_request.end())
         throw std::runtime_error("Dependency '" + s.target_path() + "' doesn't have version specified");

      dependency_t d;
      d.public_id_ = string(s.target_path().begin() + 1, s.target_path().end());
      d.version_ = (**i).value();

      dependencies.push_back(d);
   }

   return dependencies;
}

template<typename Digest>
class digest_filter {
   public:
      typedef char char_type;
      struct category : io::output_filter_tag, io::multichar_tag, io::optimally_buffered_tag {};

      std::streamsize optimal_buffer_size() const { return 0; }

      template<typename Sink>
      std::streamsize write(Sink& snk, const char_type* s, std::streamsize n)
      {
         digest_.input(s, n);
         return io::write(snk, s, n);
      }

      string to_string() { return digest_.to_string(); }

   private:
      Digest digest_;
};

BOOST_IOSTREAMS_PIPABLE(digest_filter, 1)

static
string calculate_md5(const fs::path& filename)
{
   typedef digest_filter<boost::crypto::md5> md5_filter;

   fs::ifstream digest_data(filename);
   io::filtering_ostream output(md5_filter() | io::null_sink());

   io::copy(digest_data, output);

   return output.component<md5_filter>(0)->to_string();
}

static const string file_schema("file://");
static
string extract_filepath_from_url(const string& url)
{
   if (url.find(file_schema) != 0)
      throw std::runtime_error("Warehouse url'" + url + "' should be 'file://...' type to be able to add packages");

   return url.substr(file_schema.size());
}

static
list<string>
gather_targets(const project& p,
               const string& prefix) {
   list<string> result;
   unordered_set<string> implicit_targets;
   unordered_set<string> explicit_targets;

   for (const auto& bt : p.targets()) {
      if (bt.second->is_local())
         continue;

      auto& target_name = bt.second->name();
      if (bt.second->is_explicit()) {
         if (explicit_targets.find(target_name) == explicit_targets.end()) {
            explicit_targets.insert(target_name);
            result.push_back(prefix + target_name);
         }
      } else {
         if (implicit_targets.find(target_name) == implicit_targets.end()) {
            implicit_targets.insert(target_name);
            result.push_back("@" + prefix + target_name);
         }
      }
   }

   for (auto& alias : p.aliases()) {
      string alias_prefix = [&] {
         if (alias.alias_.empty())
            return prefix;

         if (prefix.empty())
            return alias.alias_.string() + "/";
         else
            return prefix + "/" + alias.alias_.string();
      }();
      result.merge(gather_targets(p.get_engine().load_project(alias.full_fs_path_), alias_prefix));
   }

   return result;
}

static
vector<string>
gather_targets(const project& p) {
   auto targets = gather_targets(p, {});
   if (targets.empty())
      throw std::runtime_error("There is no targets to export");

   return {targets.begin(), targets.end()};
}

void warehouse_impl::add_to_packages(const project& p)
{
   if (p.name().empty())
      throw std::runtime_error("Project must have id to be exported");

   const feature_set* build_request = p.get_engine().feature_registry().make_set();
   feature_set* project_requirements  = p.get_engine().feature_registry().make_set();
   p.local_requirements().eval(*build_request, project_requirements);
   feature_set::const_iterator i_version = project_requirements->find("version");
   if (i_version == project_requirements->end())
      throw std::runtime_error("Project must have explicitly specified version to be exported");

   const location_t packages_db_root = extract_filepath_from_url(repository_url_);
   fs::path packages_db_full_path = packages_db_root / packages_filename;
   if (!exists(packages_db_full_path)) {
      fs::ofstream f(packages_db_full_path, ios_base::trunc);
      if (!f)
         throw std::runtime_error("Can't create '" + packages_db_full_path.string() + "'");
      f << "[]";
   }

   packages_t packages = load_packages(packages_db_full_path);

   const string version = (**i_version).value();
   const string public_id = p.name();

   package_t package;
   package.public_id_ = public_id;
   package.version_ = version;
   package.filename_ = boost::algorithm::replace_all_copy(public_id, "/", ".") + "-" + version + ".tar.bz2";

   const fs::path package_full_path = packages_db_root / package.filename_;
   make_package_archive(p.location(), package_full_path);
   package.md5_ = calculate_md5(package_full_path);
   package.filesize_ = file_size(package_full_path);
   package.dependencies_ = gather_dependencies(p);
   package.targets_ =  gather_targets(p);

   packages_t::iterator i = find_package(packages, public_id, version);
   if (i == packages.end())
      packages.insert(make_pair(public_id, package));
   else
      i->second = package;

   write_packages(packages_db_full_path, packages);
}

warehouse::versions_t
warehouse_impl::get_package_versions(const string& public_id) const
{
   auto versions = packages_.equal_range(public_id);
   versions_t result;
   for(; versions.first != versions.second; ++versions.first)
      result.push_back({versions.first->second.version_, versions.first->second.targets_});

   return result;
}

std::vector<std::string>
warehouse_impl::get_installed_versions(const std::string& public_id) const
{
   static const boost::regex pattern("^use-project.*<version>(.*);");

   vector<string> result;
   const fs::path project_root = storage_dir_ / "libs" / public_id / "hamfile";
   if (!exists(project_root))
      return result;

   fs::ifstream f{project_root};
   string line;
   boost::smatch m;
   while (getline(f, line)) {
      if (boost::regex_match(line, m, pattern))
         result.push_back(m[1]);
   }

   sort(result.begin(), result.end());

   return result;
}

bool warehouse_impl::already_materialized(const location_t& public_id) const
{
   return fs::exists(storage_dir_ / "libs" / public_id);
}

static
void remove_alias_from_package_hamfile(const string& package_public_id,
                                       const string& package_version,
                                       const vector<string>& package_targets,
                                       const fs::path& path_to_hamfile)
{
   if (!package_targets.empty())
      throw std::runtime_error("Can't remove package with non-empty targets");

   const fs::path tmp_hamfile = path_to_hamfile.branch_path() / (path_to_hamfile.filename().string() + ".tmp");
   fs::ofstream tmp_f(tmp_hamfile);
   fs::ifstream current_f(path_to_hamfile);

   string line;
   const string alias_line = make_package_proxy_line(package_version);
   while(getline(current_f, line)) {
      if (line != alias_line)
         tmp_f << line << endl;
   }

   tmp_f.close();
   current_f.close();

   fs::remove(path_to_hamfile);
   fs::rename(tmp_hamfile, path_to_hamfile);
}

void warehouse_impl::remove_package(const package_t& package_to_remove)
{
   const fs::path libs_path = storage_dir_ / "libs";
   const fs::path lib_path = libs_path / package_to_remove.public_id_;
   const fs::path package_root = lib_path / package_to_remove.version_;

   remove_alias_from_package_hamfile(package_to_remove.public_id_, package_to_remove.version_, package_to_remove.targets_, lib_path / "hamfile");
   fs::remove_all(package_root);
   fs::remove(storage_dir_ / "downloads" / package_to_remove.filename_);
}

void warehouse_impl::update_package(engine& e,
                                    package_t& package_to_update)
{
   remove_package(package_to_update);

   package_info pi;
   pi.name_ = package_to_update.public_id_;
   pi.version_ = package_to_update.version_;

   null_warehouse_download_and_install dl_notifier;
   download_and_install(e, {pi}, dl_notifier);

   package_to_update.need_update_ = false;
}

}
