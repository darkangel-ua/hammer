#pragma once
#include <memory>
#include <vector>
#include <map>
#include <hammer/core/location.h>
#include <hammer/core/basic_meta_target.h>
#include <hammer/core/feature_registry.h>

namespace hammer {

class engine;

class project {
   public:
      struct selected_target {
         selected_target(const basic_meta_target* t,
                         const feature_set* resolved_requirements,
                         unsigned resolved_requirements_rank)
                        : target_(t),
                          resolved_requirements_(resolved_requirements),
                          resolved_requirements_rank_(resolved_requirements_rank)
         {}

         selected_target()
            : target_(nullptr),
              resolved_requirements_(nullptr)
         {}

         const basic_meta_target* target_;
         const feature_set* resolved_requirements_;
         unsigned resolved_requirements_rank_;
      };

      typedef std::multimap<std::string /* target name */, std::unique_ptr<basic_meta_target>> targets_t;
      typedef std::vector<selected_target> selected_targets_t;

      project(engine* e,
              const std::string& name,
              const location_t& location,
              const requirements_decl& req,
              const requirements_decl& usage_req);

      project(engine* e) : engine_(e) {}
      virtual ~project();

      const std::string& name() const { return name_; }
      void name(const std::string& new_name) { name_ = new_name; }

      const location_t& location() const { return location_; }
      void location(const location_t& l);

      const requirements_decl& usage_requirements() const { return usage_requirements_; }
      const requirements_decl& requirements() const { return requirements_; }

      requirements_decl& usage_requirements() { return usage_requirements_; }
      requirements_decl& requirements() { return requirements_; }

      void add_target(std::unique_ptr<basic_meta_target> t);
      const targets_t& targets() const { return targets_; }

      // FIXME: there is can be many metatargets with same name
      const basic_meta_target* find_target(const std::string& name) const;
      // FIXME: there is can be many metatargets with same name
      basic_meta_target* find_target(const std::string& name);

      hammer::engine* get_engine() const { return engine_; }
      const location_t& intermediate_dir() const { return intermediate_dir_; }
      feature_registry& local_feature_registry() const { return local_feature_registry_; }
      bool is_root() const { return is_root_; }
      void set_root(bool v) { is_root_ = v; }

      void instantiate(const std::string& target_name,
                       const feature_set& build_request,
                       std::vector<basic_target*>* result) const;
      bool operator == (const project& rhs) const;
      bool operator != (const project& rhs) const { return !(*this == rhs); }

      // select targets in project that satisfied build_request. Can return empty list.
      selected_targets_t select_best_alternative(const feature_set& build_request) const;

      // choose best alternative for target_name satisfied build_request
      selected_target select_best_alternative(const std::string& target_name, const feature_set& build_request, const bool allow_locals = false) const;
      selected_target try_select_best_alternative(const std::string& target_name, const feature_set& build_request, const bool allow_locals = false) const;
      feature_set* try_resolve_local_features(const feature_set& fs) const;

   private:
      hammer::engine* engine_;
      std::string name_;
      requirements_decl requirements_;
      requirements_decl usage_requirements_;
      location_t location_;
      targets_t targets_;
      location_t intermediate_dir_;
      bool is_root_ = false;
      mutable feature_registry local_feature_registry_;
};

}
