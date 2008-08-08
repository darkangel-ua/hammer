#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>

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

   class engine : boost::noncopyable
   {
      public:
         engine();
         project& load_project(location_t project_path, const project& from_project);
         project& load_project(location_t project_path);
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
         struct project_alias_data
         {
            location_t location_;
            const feature_set* properties_;
         };

         typedef std::map<location_t, project_alias_data> global_project_links_t;

         projects_t projects_;
         global_project_links_t global_project_links_;

         boost::shared_ptr<type_registry> type_registry_;
         pool pool_;
         hammer::feature_registry* feature_registry_;
         hammer::call_resolver resolver_;
         boost::shared_ptr<generator_registry> generators_;
         boost::shared_ptr<scm_manager> scm_manager_;

         project* get_upper_project(const location_t& project_path);
         location_t resolve_project_alias(const location_t& loc, 
                                          project_alias_data& alias_data) const;
         void initial_materialization(const project_alias_data& alias_data) const;

         void project_rule(project* p, std::vector<pstring>& name, project_requirements_decl* req, project_requirements_decl* usage_req);
         void lib_rule(project* p, std::vector<pstring>& name, sources_decl* sources, 
                       requirements_decl* fs, feature_set* default_build, requirements_decl* usage_requirements);
         void exe_rule(project* p, std::vector<pstring>& name, sources_decl& sources, requirements_decl* fs, 
                       feature_set* default_build, requirements_decl* usage_requirements);
         void obj_rule(project* p, pstring& name, sources_decl& sources, requirements_decl* fs,
                       feature_set* default_build, requirements_decl* usage_requirements);
         void alias_rule(project* p, 
                         pstring& name, 
                         sources_decl& sources, 
                         requirements_decl* fs, 
                         feature_set* default_build, 
                         requirements_decl* usage_requirements);
         void import_rule(project* p, std::vector<pstring>& name);
         void feature_feature_rule(project* p, std::vector<pstring>& name, std::vector<pstring>* values,
                                   std::vector<pstring>& attributes);
         void feature_compose_rule(project* p, feature& f, feature_set& components);
         sources_decl glob_rule(project* p, std::vector<pstring>& patterns, std::vector<pstring>* exceptions);
         void explicit_rule(project* p, const pstring& target_name);
         void use_project_rule(project* p, const pstring& project_id_alias, 
                               const pstring& project_location, feature_set* props);
   };

   boost::filesystem::path find_root(const boost::filesystem::path& initial_path);
}