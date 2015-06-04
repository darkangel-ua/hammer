#ifndef a84a0ac54_505f_4a03_bb2d_bb33db380037
#define a84a0ac54_505f_4a03_bb2d_bb33db380037

#include <hammer/core/warehouse.h>
#include <boost/filesystem/path.hpp>
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
      bool has_project(const location_t& project_path) const;
      boost::shared_ptr<project> load_project(const location_t& project_path);
      std::vector<package_info> get_unresoved_targets_info(const std::vector<const warehouse_target*>& targets) const;
      void download_and_install(const std::vector<package_info>& packages);

   protected:
      void init_impl(const std::string& url);

   private:
      struct gramma;
      struct package_t
      {
         package_t() : filesize_(0) {}

         std::string filename_;
         unsigned long filesize_;
         std::string version_;
         std::string name_;
         std::string public_id_;
      };

      typedef std::multimap<std::string, package_t> packages_t;

      engine& engine_;
      boost::filesystem::path repository_path_;
      std::string repository_url_;
      packages_t packages_;

      packages_t::iterator find_package(const std::string& public_id,
                                        const std::string& version);
      void download_package(const package_t& p,
                            const boost::filesystem::path& working_dir);
      void install_package(const package_t& p,
                           const boost::filesystem::path& working_dir);
};

}

#endif
