#pragma once
#include <vector>
#include <yaml-cpp/yaml.h>
#include <hammer/core/location.h>

namespace hammer {

class engine;
class project;
class basic_target;
class warehouse_target;
class call_resolver;
class iwarehouse_download_and_install;

class warehouse : public boost::noncopyable {
   public:
      // FIXME: we need to represent some form of unknown package
      struct package_info {
         std::string name_;
         std::string version_;
         size_t package_file_size_;
      };

      struct version_info {
         std::string version_;
         std::vector<std::string> targets_;
      };

      typedef std::vector<package_info> package_infos_t;
      typedef std::vector<version_info> versions_t;

      warehouse(const std::string& id,
                const boost::filesystem::path& storage_dir);
      virtual ~warehouse() {}

      // check if project belongs to warehouse
      virtual bool owned(const project& p) const = 0;
      // update package database
      package_infos_t update() { return update_impl(); }
      // update all packages that has been changed on the server, but stil not updated localy
      void update_all_packages(engine& e) { return update_all_packages_impl(e); }

      static const std::string any_version;
      // use any_version to look up project regardless of version
      virtual bool has_project(const location_t& project_path,
                               const std::string& version) const = 0;

      // FIXME: we need to represent some form of unknown package
      // possibly by introducing something like pair<known packages/unknown packages> or separate class
      virtual
      package_infos_t
      get_unresoved_targets_info(engine& e,
                                 const std::vector<const warehouse_target*>& targets) const = 0;
      virtual
      std::vector<std::string>
      get_installed_versions(const std::string& public_id) const = 0;

      virtual void download_and_install(engine& e,
                                        const std::vector<package_info>& packages,
                                        iwarehouse_download_and_install& notifier) = 0;
      // FIXME: this is temporal hack. I will remove this when web part will be implemented
      virtual void add_to_packages(const project& p) = 0;
      virtual versions_t get_package_versions(const std::string& public_id) const = 0;

      // Generic information about warehouse in hierarchical form
      virtual
      YAML::Node
      info() const = 0;

   public:
      const std::string id_;
      const boost::filesystem::path storage_dir_;

   private:
      virtual package_infos_t update_impl() = 0;
      virtual void update_all_packages_impl(engine& e) = 0;
};

class iwarehouse_download_and_install {
   public:
      /// \brief return false if we want to interrupt
      virtual bool on_download_begin(const std::size_t index,
                                     const warehouse::package_info& package) = 0;
      virtual void on_download_end(const std::size_t index,
                                   const warehouse::package_info& package) = 0;

      /// \brief return false if we want to interrupt
      virtual bool on_install_begin(const std::size_t index,
                                    const warehouse::package_info& package) = 0;
      virtual void on_install_end(const std::size_t index,
                                  const warehouse::package_info& package) = 0;

      virtual ~iwarehouse_download_and_install() {}
};

class null_warehouse_download_and_install : public iwarehouse_download_and_install {
   public:
      bool on_download_begin(const std::size_t index,
                             const warehouse::package_info& package) override { return true; }
      void on_download_end(const std::size_t index,
                           const warehouse::package_info& package) override {}
      bool on_install_begin(const std::size_t index,
                            const warehouse::package_info& package) override { return true; }
      void on_install_end(const std::size_t index,
                          const warehouse::package_info& package) override {}
};

std::vector<const warehouse_target*>
find_all_warehouse_unresolved_targets(const std::vector<basic_target*>& targets);

void install_warehouse_rules(hammer::engine& engine);
void add_traps(warehouse& wh,
               project& p,
               const std::string& public_id,
               const std::string& internal_path);
}
