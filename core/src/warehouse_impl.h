#ifndef a84a0ac54_505f_4a03_bb2d_bb33db380037
#define a84a0ac54_505f_4a03_bb2d_bb33db380037

#include <hammer/core/warehouse.h>
#include <boost/filesystem/path.hpp>
#include <boost/unordered_map.hpp>
#include <map>

namespace hammer {

class main_target;
class feature_set;
class basic_target;

class warehouse_impl : public warehouse
{
   public:
      warehouse_impl(engine& e);
      ~warehouse_impl();
      bool project_from_warehouse(const project& p) const override;
      bool has_project(const location_t& project_path,
                       const std::string& version) const override;
      boost::shared_ptr<project> load_project(const location_t& project_path) override;
      std::vector<package_info> get_unresoved_targets_info(const std::vector<const warehouse_target*>& targets) const override;
      void download_and_install(const std::vector<package_info>& packages) override;

      void add_to_packages(const project& p,
                           const location_t& packages_db_root_ = location_t()) override;
      versions_t get_package_versions(const std::string& public_id) const override;


   protected:
      void init_impl(const std::string& url,
                     const std::string& storage_dir) override;
      package_infos_t update_impl() override;
      void update_all_packages_impl() override;

   private:
      struct gramma;
      struct dependency_t
      {
         std::string version_;
         std::string public_id_;
      };

      struct package_t
      {
         std::string filename_;
         unsigned long filesize_ = 0;
         std::string md5_;
         std::string version_;
         std::string public_id_;
         bool need_update_ = false;
         std::vector<dependency_t> dependencies_;
      };

      typedef std::multimap<std::string, package_t> packages_t;
      typedef boost::unordered_map<std::string, dependency_t> unresolved_dependencies_t;
      typedef boost::unordered_map<std::string, package_info> unresolved_packages_t;

      engine& engine_;
      boost::filesystem::path repository_path_;
      std::string repository_url_;
      packages_t packages_;

      static
      packages_t::iterator find_package(packages_t& packages,
                                        const std::string& public_id,
                                        const std::string& version);
      packages_t::iterator find_package(const std::string& public_id,
                                        const std::string& version);
      packages_t::const_iterator find_package(const std::string& public_id,
                                              const std::string& version) const;
      void download_package(const package_t& p,
                            const boost::filesystem::path& working_dir);
      void install_package(const package_t& p,
                           const boost::filesystem::path& working_dir);
      void update_package(package_t& package_to_update);
      void remove_package(const package_t& package_to_remove);

      static
      packages_t load_packages(const boost::filesystem::path& filepath);
      void resolve_dependency(unresolved_packages_t& packages,
                              const dependency_t& d,
                              const project& repository_project) const;
      static warehouse::package_info to_package_info(const package_t& p);
      bool resolves_to_real_project(const std::string& public_id,
                                    const project& repository_project);
      static
      void write_packages(const location_t& packages_db_path,
                          const packages_t& packages);

      static
      std::vector<dependency_t> gather_dependencies(const project& p);
};

}

#endif
