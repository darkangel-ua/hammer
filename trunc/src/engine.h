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

   class engine : boost::noncopyable
   {
      public:
         engine(const boost::filesystem::path& root_path);
         const project& load_project(const location_t& project_path);
         void insert(project* p);
         type_registry& get_type_registry() { return *type_registry_; }
         pool& pstring_pool() { return pool_; }
         pool& targets_pool() { return pool_; }
         const location_t& root() const { return root_path_; }
         hammer::feature_registry& feature_registry() { return *feature_registry_; }
         void generate(basic_target* t);     
         ~engine();

      private:
         typedef boost::ptr_map<const location_t, project> projects_t;

         boost::filesystem::path root_path_;
         projects_t projects_;
         boost::shared_ptr<type_registry> type_registry_;
         pool pool_;
         hammer::feature_registry* feature_registry_;
         call_resolver resolver_;

         void project_rule(project* p, std::vector<pstring>& name);
         void lib_rule(project*p, std::vector<pstring>& name, std::vector<pstring>& sources, feature_set* fs);
         void exe_rule(project*p, std::vector<pstring>& name, std::vector<pstring>& sources, feature_set* fs);
   };

   boost::filesystem::path find_root(const boost::filesystem::path& initial_path);
}