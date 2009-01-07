#if !defined(h_4b62efd8_cafb_41d3_a747_e981804129e5)
#define h_4b62efd8_cafb_41d3_a747_e981804129e5

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <deque>
#include "location.h"
#include "project.h"
#include "pool.h"
#include "call_resolver.h"

namespace hammer
{
   class basic_target;
   class type_registry;
   class feature_registry;
   class generator_registry;
   class feature;
   class project_requirements_decl;
   class scm_manager;
   class scm_client;

   class engine : boost::noncopyable
   {
      public:
         class loaded_projects_t;

         engine();
         loaded_projects_t load_project(location_t project_path, const project& from_project);
         project& load_project(location_t project_path);
         void load_hammer_script(location_t filepath);

         void insert(project* p);
         type_registry& get_type_registry() { return *type_registry_; }
         pool& pstring_pool() { return pool_; }
         pool& targets_pool() { return pool_; }
         generator_registry& generators() const { return *generators_; }
         hammer::feature_registry& feature_registry() { return *feature_registry_; }
         hammer::call_resolver& call_resolver() { return resolver_; }
         ~engine();

      private:
         typedef boost::ptr_map<const location_t, project> projects_t;
         struct project_alias_node;
         typedef boost::ptr_map<location_t, project_alias_node> global_project_links_t;

         struct project_alias_data
         {
            location_t location_;
            const feature_set* properties_;
         };
         
         struct project_alias_node
         {
            typedef std::vector<project_alias_data> aliases_data_t;

            // mapping associated with this node. One project symlink path such as /foo/bar 
            // can be mapped on many different filesystem paths
            aliases_data_t aliases_data_; 
            
            global_project_links_t project_symlinks_;
         };

         struct resolved_project_symlink_t
         {
            resolved_project_symlink_t(const location_t& tail, 
                                       const project_alias_node::aliases_data_t& symlinks_data) 
                                       : tail_(tail),
                                         symlinks_data_(&symlinks_data)
            {
            }

            // Unresolved part. bar is a tail_, if /foo/bar was requested and foo was founded.
            location_t tail_;
            const project_alias_node::aliases_data_t* symlinks_data_; // cannot be NULL
         };
         typedef std::vector<resolved_project_symlink_t> resolved_project_symlinks_t;

         struct repository_data
         {
            repository_data(const project* defined_in_project, 
                            const location_t& location,
                            const feature_set* properties)
                           :
                            defined_in_project_(defined_in_project),
                            location_(location),
                            properties_(properties),
                            materialized_(false)
            {}

            const project* defined_in_project_;
            location_t location_;
            const feature_set* properties_;
            bool materialized_;
         };

         typedef std::map<const project*, std::map<location_t /* alias */, std::string /* map to */> > use_project_data_t;
         typedef std::deque<repository_data> repositories_t;

         projects_t projects_;
         global_project_links_t global_project_links_;
         
         boost::shared_ptr<type_registry> type_registry_;
         pool pool_;
         hammer::feature_registry* feature_registry_;
         hammer::call_resolver resolver_;
         boost::shared_ptr<generator_registry> generators_;
         boost::shared_ptr<scm_manager> scm_manager_;
         use_project_data_t use_project_data_;
         repositories_t repositories_;

         loaded_projects_t try_load_project(location_t project_path, const project& from_project);
         loaded_projects_t try_load_project(const location_t& tail_path, const project_alias_data& symlink);
         loaded_projects_t try_load_project(location_t project_path);
         void update_project_scm_info(project& p, const project_alias_data& alias_data) const;
         project* get_upper_project(const location_t& project_path);
         bool materialize_or_load_next_repository();
         void resolve_project_alias(resolved_project_symlinks_t& symlinks,
                                    const location_t& project_symlink);
         void resolve_project_alias(resolved_project_symlinks_t& symlinks,
                                    location_t::const_iterator first, location_t::const_iterator last,
                                    global_project_links_t& symlink_storage);
         void resolve_use_project(location_t& resolved_use_path, location_t& tail_path,
                                  const hammer::project& project, const location_t& path_to_resolve);
         const scm_client* try_resolve_scm_client(const project& p);
         const scm_client& resolve_scm_client(const project& p);
         const scm_client& resolve_scm_client_impl(const project& p);
         void initial_materialization(const project_alias_data& alias_data) const;
         bool try_materialize_project(const location_t& project_path, 
                                      const project& upper_project);
         const project* find_upper_materialized_project(const project& p);
         const project* find_upper_materialized_project(const location_t& location);

         void project_rule(project* p, std::vector<pstring>& name, project_requirements_decl* req, project_requirements_decl* usage_req);
         void lib_rule(project* p, std::vector<pstring>& name, sources_decl* sources, 
                       requirements_decl* fs, feature_set* default_build, requirements_decl* usage_requirements);
         void header_lib_rule(project* p, std::vector<pstring>& name, sources_decl* sources, 
                              requirements_decl* fs, feature_set* default_build, requirements_decl* usage_requirements);
         void exe_rule(project* p, std::vector<pstring>& name, sources_decl& sources, requirements_decl* fs, 
                       feature_set* default_build, requirements_decl* usage_requirements);
         void obj_rule(project* p, pstring& name, sources_decl& sources, requirements_decl* fs,
                       feature_set* default_build, requirements_decl* usage_requirements);
         void pch_rule(project* p, pstring& name, sources_decl& sources, requirements_decl* fs,
                       feature_set* default_build, requirements_decl* usage_requirements);
         void alias_rule(project* p, 
                         pstring& name, 
                         sources_decl* sources, 
                         requirements_decl* fs, 
                         feature_set* default_build, 
                         requirements_decl* usage_requirements);
         sources_decl testing_run_rule(project* p, 
                                       sources_decl* sources, 
                                       std::vector<pstring>* args, 
                                       std::vector<pstring>* input_files, 
                                       requirements_decl* requirements,
                                       pstring* target_name);
         void copy_rule(project* p, pstring& name, sources_decl& sources, requirements_decl* fs,
                        feature_set* default_build, requirements_decl* usage_requirements);
         void import_rule(project* p, std::vector<pstring>& name);
         void feature_feature_rule(project* p, std::vector<pstring>& name, std::vector<pstring>* values,
                                   std::vector<pstring>* attributes);
         void feature_compose_rule(project* p, feature& f, feature_set& components);
         void variant_rule(project* p, pstring& variant_name, pstring* base, feature_set& components);
         sources_decl glob_rule(project* p, std::vector<pstring>& patterns, 
                                std::vector<pstring>* exceptions, bool recursive);
         void explicit_rule(project* p, const pstring& target_name);
         void use_project_rule(project* p, const pstring& project_id_alias, 
                               const pstring& project_location, feature_set* props);
         void repository_rule(project* p, const pstring& project_location, feature_set* props);
   };

   boost::filesystem::path find_root(const boost::filesystem::path& initial_path);

   class engine::loaded_projects_t
   {
      public:
         friend class engine;

         explicit loaded_projects_t(project* v) : projects_(1, v) {}
         void push_back(project* v) { projects_.push_back(v); }
         project& front() const { return *projects_.front(); }
         bool is_single() const { return projects_.size() == 1; }
         loaded_projects_t& operator +=(const loaded_projects_t& rhs) 
         {
            projects_.insert(projects_.end(), rhs.projects_.begin(), rhs.projects_.end());
            return *this;
         }
         project::selected_targets_t select_best_alternative(const feature_set& build_request) const;
         const basic_meta_target* select_best_alternative(const pstring& target_name, const feature_set& build_request) const;

         
      private:      
         typedef std::vector<project*> projects_t;
         
         projects_t projects_;

         loaded_projects_t() {};
         bool empty() const { return projects_.empty(); }
         void post_process(project::selected_targets_t& result, const feature_set& build_request) const;
   };
}

#endif //h_4b62efd8_cafb_41d3_a747_e981804129e5
