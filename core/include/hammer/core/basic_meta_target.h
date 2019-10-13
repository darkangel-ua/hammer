#pragma once
#include <vector>
#include <hammer/core/requirements_decl.h>
#include <hammer/core/location.h>
#include <hammer/core/sources_decl.h>

namespace hammer {

class feature_set;
class project;
class engine;
class basic_target;
class main_target;
class type_registry;
class basic_meta_target;
class instantiation_context;

typedef std::vector<std::pair<const basic_meta_target* /* target */,
                              const feature_set* /* requested build properties*/> > meta_targets_t;

class basic_meta_target {
   public:
      basic_meta_target(project* p,
                        const std::string& name,
                        const requirements_decl& req,
                        const requirements_decl& usage_req);
      basic_meta_target() {}
      virtual ~basic_meta_target();

      const project&
      get_project() const { return *project_; }

      engine&
      get_engine() const;

      const std::string&
      name() const { return name_; }

      void name(const std::string& v) { name_ = v; }
      void add_sources(const sources_decl& s);

      const sources_decl&
      sources() const { return sources_; }
      void sources(const sources_decl& s);

      const requirements_decl&
      usage_requirements() const { return usage_requirements_; }

      const requirements_decl&
      requirements() const { return requirements_; }

      requirements_decl&
      usage_requirements() { return usage_requirements_; }

      requirements_decl&
      requirements() { return requirements_; }

      void usage_requirements(const requirements_decl& r) { usage_requirements_ = r; }
      void requirements(const requirements_decl& r) { requirements_ = r; }

      void default_build(const requirements_decl& v);
      const requirements_decl&
      default_build() const { return default_build_; }

      virtual
      const location_t&
      location() const;

      void split_sources(sources_decl* simple_targets,
                         meta_targets_t* meta_targets,
                         const sources_decl& sources,
                         const feature_set& build_request) const;

      void instantiate(instantiation_context& ctx,
                       const main_target* owner,
                       const feature_set& build_request,
                       std::vector<basic_target*>* result,
                       feature_set* usage_requirements) const;
      void clear_instantiation_cache();

      void set_explicit(bool v) { is_explicit_ = v; }
      bool is_explicit() const { return is_explicit_; }

      void set_local(bool v) { is_local_ = v; }
      bool is_local() const { return is_local_; }

   protected:
      // returns true if instantiate() can cache instantiation for build_request
      virtual
      bool is_cachable(const main_target* owner) const { return true; }

      virtual
      void instantiate_impl(instantiation_context& ctx,
                            const main_target* owner,
                            const feature_set& build_request,
                            std::vector<basic_target*>* result,
                            feature_set* usage_requirements) const = 0;

      void instantiate_simple_targets(const sources_decl& targets,
                                      const feature_set& build_request,
                                      const main_target& owner,
                                      std::vector<basic_target*>* result) const;
      void split_one_source(sources_decl* simple_targets,
                            meta_targets_t* meta_targets,
                            const source_decl& source,
                            const feature_set& build_request,
                            const type_registry& tr) const;
      void resolve_meta_target_source(const source_decl& source,
                                      const feature_set& build_request,
                                      meta_targets_t* meta_targets) const;

      virtual
      basic_target*
      create_simple_target(const main_target& owner,
                           const location_t& source_location,
                           const target_type& tp,
                           const feature_set* properties) const;

   private:
      struct cached_instantiation_data_t {
         const feature_set* build_request_;
         std::vector<basic_target*> instantiated_targets_;
         feature_set* computed_usage_requirements_;
      };
      typedef std::vector<cached_instantiation_data_t> instantiation_cache_t;

      hammer::project* project_;
      std::string name_;
      sources_decl sources_;
      requirements_decl requirements_;
      requirements_decl usage_requirements_;
      requirements_decl default_build_;
      bool is_explicit_;
      bool is_local_ = false;
      mutable instantiation_cache_t instantiation_cache_;
};

void instantiate_meta_targets(instantiation_context& ctx,
                              const meta_targets_t& targets,
                              const main_target* owner,
                              std::vector<basic_target*>* result,
                              feature_set* usage_requirments);

// we need to convert source_decl from just 'foo' form to './/foo' form
// this will helps 'extract_sources' to correctly adjust target_path later
void adjust_dependency_features_sources(feature_set& set_to_adjust);

}
