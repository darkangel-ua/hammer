#ifndef ad250e1ad_1eb7_49c2_ba1f_dea30727a139
#define ad250e1ad_1eb7_49c2_ba1f_dea30727a139

#include <hammer/core/location.h>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace hammer {

   class engine;
   class project;
   class basic_target;
   class warehouse_target;
   class call_resolver;

   class warehouse : public boost::noncopyable
   {
      public:
         // FIXME: we need to represent some form of unknown package
         struct package_info
         {
            std::string name_;
            std::string version_;
            size_t package_file_size_;
         };

         virtual ~warehouse() {}
         void init(const std::string& url,
                   const std::string& storage_dir)
         {
            init_impl(url, storage_dir);
         }

         // update package database
         void update() { update_impl(); }
         virtual bool has_project(const location_t& project_path) const = 0;
         virtual boost::shared_ptr<project> load_project(const location_t& project_path) = 0;
         // FIXME: we need to represent some form of unknown package
         // posibly by introducing something like pair<known packages/unknown packages> or separate class
         virtual
         std::vector<package_info> get_unresoved_targets_info(const std::vector<const warehouse_target*>& targets) const = 0;
         virtual void download_and_install(const std::vector<package_info>& packages) = 0;
         // FIXME: this is temporal hack. I will remove this when web part will be implemented
         virtual void add_to_packages(const project& p,
                                      const location_t& packages_db_path) = 0;

      private:
         virtual void init_impl(const std::string& url,
                                const std::string& storage_dir) = 0;
         virtual void update_impl() = 0;
   };

   std::vector<const warehouse_target*>
   find_all_warehouse_unresolved_targets(const std::vector<basic_target*>& targets);

   void install_warehouse_rules(call_resolver& resolver,
                                engine& engine);
}

#endif
