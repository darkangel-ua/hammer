#pragma once
#include <memory>
#include <vector>
#include <map>
#include <boost/regex.hpp>
#include <hammer/core/location.h>
#include <hammer/core/basic_meta_target.h>
#include <hammer/core/feature_registry.h>

namespace hammer {

class engine;
class loaded_projects;
class build_request;

class project : public boost::noncopyable {
   public:
      struct selected_target {
         selected_target(const basic_meta_target* t,
                         const feature_set* resolved_requirements,
                         unsigned resolved_requirements_rank,
                         const feature_set* resolved_build_request)
                        : target_(t),
                          resolved_requirements_(resolved_requirements),
                          resolved_requirements_rank_(resolved_requirements_rank),
                          resolved_build_request_(resolved_build_request)
         {}

         selected_target()
            : target_(nullptr),
              resolved_requirements_(nullptr),
              resolved_build_request_(nullptr)
         {}

         const basic_meta_target* target_;
         const feature_set* resolved_requirements_;
         unsigned resolved_requirements_rank_;
         const feature_set* resolved_build_request_;
      };

      typedef std::multimap<std::string /* target name */, std::unique_ptr<basic_meta_target>> targets_t;
      typedef std::vector<selected_target> selected_targets_t;

      struct alias {
         enum class match {
            always,
            exact
         };

         boost::filesystem::path alias_;
         boost::filesystem::path full_fs_path_;
         const feature_set* requirements_;
         match match_strategy_;

         bool is_transparent() const { return alias_.empty(); }
      };
      using aliases_t = std::vector<alias>;

      struct dependency {
         boost::regex target_ref_mask_;
         feature_set* properties_;
      };

      using dependencies_t = std::vector<dependency>;

      project(engine& e,
              const project* parent,
              const std::string& name,
              const location_t& location,
              const requirements_decl& local_req,
              const requirements_decl& local_usage_req,
              const dependencies_t& dependencies);

      project(engine& e,
              const project* parent,
              const location_t& l);
      virtual ~project();

      const std::string&
      name() const { return name_; }

      void name(const std::string& new_name) { name_ = new_name; }

      const location_t&
      location() const { return location_; }

      void local_requirements(const requirements_decl& req);
      void local_usage_requirements(const requirements_decl& req);

      const requirements_decl&
      local_requirements() const { return local_requirements_; }

      const requirements_decl&
      local_usage_requirements() const { return local_usage_requirements_; }

      const requirements_decl&
      usage_requirements() const { return usage_requirements_; }

      const requirements_decl&
      requirements() const { return requirements_; }

      const dependencies_t&
      dependencies() const { return dependencies_; }
      // both function will prepend v to parent->dependencies() and use combined
      // this will allow to calculate all replacements with one call instead of traversing up to parents
      void dependencies(const dependencies_t& v);
      void dependencies(dependencies_t&& v);

      void default_build(const feature_set& v);

      const feature_set*
      default_build() const { return default_build_; }

      bool publishable() const;

      std::string
      publishable_version() const;

      void add_target(std::unique_ptr<basic_meta_target> t);

      const targets_t&
      targets() const { return targets_; }

      // FIXME: there is can be many metatargets with same name
      const basic_meta_target*
      find_target(const std::string& name) const;

      // FIXME: there is can be many metatargets with same name
      basic_meta_target*
      find_target(const std::string& name);

      hammer::engine&
      get_engine() const { return engine_; }

      const location_t&
      intermediate_dir() const { return intermediate_dir_; }

      const hammer::feature_registry&
      feature_registry() const;

      hammer::feature_registry&
      feature_registry();

      bool is_root() const { return is_root_; }
      void set_root(bool v) { is_root_ = v; }

      virtual
      loaded_projects
      load_project(const location_t& path) const;

      void add_alias(const location_t& alias,
                     const location_t& fs_path,
                     const feature_set* requirements,
                     const alias::match match_strategy = alias::match::always);

      const aliases_t&
      aliases() const;

      void instantiate(const std::string& target_name,
                       const feature_set& build_request,
                       std::vector<basic_target*>* result) const;

      bool operator == (const project& rhs) const { return this == &rhs; }
      bool operator != (const project& rhs) const { return !(*this == rhs); }

      // select targets in project that satisfied build_request. Can return empty list
      selected_targets_t
      try_select_best_alternative(const build_request& build_request) const;
      selected_targets_t
      try_select_best_alternative(const feature_set& build_request) const;

      // choose best alternative for target_name satisfied build_request
      selected_target
      select_best_alternative(const std::string& target_name,
                              const build_request& build_request,
                              const bool allow_locals = false) const;
      selected_target
      try_select_best_alternative(const std::string& target_name,
                                  const build_request& build_request,
                                  const bool allow_locals = false) const;
      selected_target
      try_select_best_alternative(const std::string& target_name,
                                  const feature_set& build_request,
                                  const bool allow_locals = false) const;

   public:
      const project* const parent_;

   private:
      struct aliases_impl;
      friend struct aliases_impl;

      hammer::engine& engine_;
      std::string name_;
      // this is local to project requirements
      requirements_decl local_requirements_;
      // this is merged parent + local project requirements
      requirements_decl requirements_;
      // this is local to project usage requirememts
      requirements_decl local_usage_requirements_;
      // this is merged parent + local usage requirements
      requirements_decl usage_requirements_;
      // this is merged parent + local default build
      feature_set* default_build_ = nullptr;
      const location_t location_;
      targets_t targets_;
      location_t intermediate_dir_;
      bool is_root_ = false;
      // mutable because we are deferring registry creation unless we want to modify it
      mutable std::shared_ptr<hammer::feature_registry> feature_registry_;
      // mutable because we cache resolved projects
      mutable std::unique_ptr<aliases_impl> aliases_;
      dependencies_t dependencies_;
};

class loaded_projects {
      typedef std::vector<const project*> projects_t;

   public:
      typedef projects_t::const_iterator const_iterator;

      loaded_projects() {}
      explicit loaded_projects(const project* v) : projects_(1, v) {}
      void push_back(const project* v);
      const_iterator begin() const { return projects_.begin(); }
      const_iterator end() const { return projects_.end(); }
      const project& front() const { return *projects_.front(); }
      bool is_single() const { return projects_.size() == 1; }

      loaded_projects&
      operator +=(const loaded_projects& rhs);

      project::selected_targets_t
      select_best_alternative(const build_request& build_request) const;

      project::selected_target
      select_best_alternative(const std::string& target_name,
                              const build_request& build_request,
                              bool allow_locals) const;

      bool empty() const { return projects_.empty(); }

   private:
      projects_t projects_;
};

const project*
find_nearest_publishable_project(const project& p);

}
