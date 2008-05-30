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

   class engine : boost::noncopyable
   {
      public:
         engine(const boost::filesystem::path& root_path);
         const project& load_project(location_t project_path);
         void insert(project* p);
         type_registry& get_type_registry() { return *type_registry_; }
         pool& pstring_pool() { return pool_; }
         pool& targets_pool() { return pool_; }
         generator_registry& generators() const { return *generators_; }
         const location_t& root() const { return root_path_; }
         hammer::feature_registry& feature_registry() { return *feature_registry_; }
         hammer::call_resolver& call_resolver() { return resolver_; }
         ~engine();

      private:
         typedef boost::ptr_map<const pstring, project> projects_t;

         boost::filesystem::path root_path_;
         projects_t projects_;
         boost::shared_ptr<type_registry> type_registry_;
         pool pool_;
         hammer::feature_registry* feature_registry_;
         hammer::call_resolver resolver_;
         boost::shared_ptr<generator_registry> generators_;

         void project_rule(project* p, std::vector<pstring>& name);
         void lib_rule(project* p, std::vector<pstring>& name, std::vector<pstring>& sources, 
                       feature_set* fs, feature_set* default_build, feature_set* usage_requirements);
         void exe_rule(project* p, std::vector<pstring>& name, std::vector<pstring>& sources, feature_set* fs, 
                       feature_set* default_build, feature_set* usage_requirements);
         void import_rule(std::vector<pstring>& name);
         void feature_feature_rule(project* p, std::vector<pstring>& name, std::vector<pstring>* values,
                                   std::vector<pstring>& attributes);
         void feature_compose_rule(project* p, feature& f, feature_set& components);
   };

   boost::filesystem::path find_root(const boost::filesystem::path& initial_path);
}